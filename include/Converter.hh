#ifndef __CONVERTER_HH
#define __CONVERTER_HH

#include <bitset>
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <sstream>
#include <string>
#include <algorithm>

#include <TFile.h>
#include <TTree.h>
#include <TTreeIndex.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TGProgressBar.h>
#include <TSystem.h>
#include <TKey.h>
#include <TROOT.h>

// Settings header
#ifndef __SETTINGS_HH
# include "Settings.hh"
#endif

// Calibration header
#ifndef __CALIBRATION_HH
# include "Calibration.hh"
#endif

// Data packets header
#ifndef __DATAPACKETS_HH
# include "DataPackets.hh"
#endif


class MiniballConverter {
	
public:
	
	MiniballConverter( std::shared_ptr<MiniballSettings> myset );
	~MiniballConverter() {};

	void MakeHists();
	void NewBuffer();
	void ResetHists();
	void MakeTree();
	void StartFile();
	void BuildMbsIndex();
	void SortDataVector();
	void SortDataMap();
	unsigned long long int SortTree( bool do_sort = true );
	static bool TimeComparator( const std::shared_ptr<MiniballDataPackets> &lhs,
							    const std::shared_ptr<MiniballDataPackets> &rhs );
	static bool MapComparator( const std::pair<unsigned long,double> &lhs,
							   const std::pair<unsigned long,double> &rhs );

	void SetOutput( std::string output_file_name );
	inline void SetOutputDirectory( std::string output_dir ){ output_dir_name = output_dir; };

	inline void CloseOutput(){
		std::cout << "\n Writing data and closing the file" << std::endl;
		output_file->Write( nullptr, TObject::kOverwrite );
		PurgeOutput();
		output_file->Close();
	};
	inline void PurgeOutput(){
		output_file->Purge(2);
	}
	inline TFile* GetFile(){ return output_file; };
	inline TTree* GetTree(){ return GetSortedTree(); };
	inline TTree* GetMbsInfo(){ return mbsinfo_tree; };
	inline TTree* GetSortedTree(){ return sorted_tree; };

	inline void AddCalibration( std::shared_ptr<MiniballCalibration> mycal ){ cal = mycal; };
	inline void SourceOnly(){ flag_source = true; };
	inline void EBISOnly(){ flag_ebis = true; };
	inline bool EBISWindow( long long int t ){
		if( ebis_period == 0 ) return false;
		else {
			// Note: (a % b) is not in range 0..b-1 for negative a.
			long long test_t = ((( t - ebis_tm_stp ) % ebis_period ) +
								ebis_period ) % ebis_period;
			return ( test_t < 4000000 && test_t > 0 );
		}
	};

	inline void AddProgressBar( std::shared_ptr<TGProgressBar> myprog ){
		prog = myprog;
		_prog_ = true;
	};


	
protected:
	
	// Flags for source or EBIS run
	bool flag_source, flag_ebis;

	// Logs
	std::stringstream sslogs;
	
	// Flag to identify Febex data words
	bool flag_febex_data0;
	bool flag_febex_data1;
	bool flag_febex_data2;
	bool flag_febex_data3;
	bool flag_febex_trace;
	bool flag_febex_info;
	
	// Interpretated variables
	unsigned long long my_good_tm_stp = 0;
	unsigned long long my_tm_stp = 0;
	unsigned long long sync_tm_stp = 0;
	long long ebis_tm_stp = 0;
	unsigned long long my_event_id;
	unsigned long my_tm_stp_lsb = 0;
	unsigned long my_tm_stp_msb = 0;
	unsigned long my_tm_stp_hsb = 0;
	unsigned long sync_tm_stp_msb = 0;
	unsigned long sync_tm_stp_hsb = 0;
	unsigned int my_info_field;
	unsigned int ebis_period = 0;
	unsigned int ebis_first = 0;
	unsigned char my_info_code;
	unsigned char my_type;
	unsigned short my_tdiff_data;
	unsigned short my_adc_data;
	unsigned short my_adc_data_lsb;
	unsigned short my_adc_data_hsb;
	unsigned int my_adc_data_int;
	unsigned char my_tag_id;
	unsigned char my_sfp_id;
	unsigned char my_board_id;
	unsigned char my_ch_id;
	unsigned char my_data_id;
	unsigned char my_trig_id;
	long my_hit_time;
	bool my_pileup;
	bool my_clip;
	bool my_flagbit;
	float my_energy;

	// For traces
	unsigned int nsamples;

	// Flag depending on the data type
	bool mbs_data, midas_data, med_data;
	
	// Flag to signify this buffer or the one before is full
	bool buffer_full = false;
	bool buffer_part = false;
	
	// Maximum size of the ADC value
	unsigned long long qmax_default;

	// Data types
	std::shared_ptr<MBSInfoPackets> mbsinfo_packet = nullptr;
	std::shared_ptr<MiniballDataPackets> write_packet = nullptr;
	std::shared_ptr<DgfData> dgf_data;
	std::shared_ptr<AdcData> adc_data;
	std::shared_ptr<FebexData> febex_data;
	std::shared_ptr<InfoData> info_data;

	// Vector for storing the data packets before time ordering
	std::vector<std::shared_ptr<MiniballDataPackets>> data_vector;
	std::vector<std::pair<unsigned long,double>> data_map;

	// Output stuff
	std::string output_dir_name;
	TFile *output_file;
	TTree *sorted_tree;
	TTree *mbsinfo_tree;

	// Counters
	std::vector<unsigned long int> ctr_dgf_hit;						// hits on each DGF module
	std::vector<unsigned long int> ctr_madc_hit;					// hits on each Mesytec ADC module
	std::vector<unsigned long int> ctr_caen_hit;					// hits on each CAEN ADC module
	std::vector<std::vector<unsigned long int>> ctr_febex_hit;		// hits on each Febex module
	std::vector<std::vector<unsigned long int>> ctr_febex_pause;   	// pause acq for module
	std::vector<std::vector<unsigned long int>> ctr_febex_resume;  	// resume acq for module
	std::vector<std::vector<unsigned long int>> ctr_febex_sync;  	// sync code from Exploder for each module
	unsigned long int jump_ctr, warp_ctr, mash_ctr;					// count timestamp jumps and warps
	unsigned long int data_ctr;										// total number of data counted
	unsigned long int reject_ctr;									// total number of reject buffers

	
	// Histograms
	std::vector<std::vector<TProfile*>> hfebex_hit;
	std::vector<std::vector<TProfile*>> hfebex_pause;
	std::vector<std::vector<TProfile*>> hfebex_resume;
	std::vector<std::vector<TProfile*>> hfebex_sync;

	std::vector<std::vector<TH1F*>> hdgf_qshort;
	std::vector<std::vector<TH1F*>> hdgf_cal;
	std::vector<std::vector<TH1F*>> hadc_qshort;
	std::vector<std::vector<TH1F*>> hadc_cal;
	std::vector<std::vector<std::vector<TH1F*>>> hfebex_qint;
	std::vector<std::vector<std::vector<TH1F*>>> hfebex_qshort;
	std::vector<std::vector<std::vector<TH1F*>>> hfebex_cal;
	std::vector<std::vector<std::vector<TH1F*>>> hfebex_mwd;
	
	TH1F *hhit_time;
	
	// Timestamp tracking
	std::vector<bool> first_data;
	std::vector<long long int> tm_stp_read;
	std::vector<std::vector<long long int>> tm_stp_febex;
	std::vector<std::vector<std::vector<long long int>>> tm_stp_febex_ch;


	// 	Settings file
	std::shared_ptr<MiniballSettings> set;

	// 	Calibrator
	std::shared_ptr<MiniballCalibration> cal;
	
	// Progress bar
	bool _prog_;
	std::shared_ptr<TGProgressBar> prog;

	// List of histograms for reset later
	TList *histlist;

};


#endif
