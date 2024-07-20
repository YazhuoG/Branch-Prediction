#ifndef SIM_BP_H
#define SIM_BP_H

typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char *            bp_name;
	unsigned int index;
    int PC_bits;
	int BHR_bits;
	int table_rows;
	int *table;
	unsigned int BHR_table;
	unsigned int PC_b;
	unsigned int PC_g;
	unsigned int BHR_g;
    int predictions;
	int mispredictions ;
}bp_params;

bp_params *bimodal = NULL, *gshare = NULL;

// Put additional data structures here as per your requirement

typedef struct bp_hybrid{
	unsigned long int K;
	unsigned long int M1;
	unsigned long int M2;
	unsigned long int N;

	unsigned int index_b;
    unsigned int index_g;
    unsigned int index_h;

	int PC_bits_b;
	int PC_bits_g;
	int BHR_bits;
	int chooser_bits;

	int Btable_rows;
	int Gtable_rows;
	int Htable_rows;

	int *table_b;
	int *table_g;
	int *table_h;

    unsigned int BHR_table;

	unsigned int PC_b;
	unsigned int PC_g;
	unsigned int PC_h;
	unsigned int BHR_g;

	int predictions;
	int mispredictions;
}bp_hybrid;

bp_hybrid *hybrid = NULL;

void init_table(bp_params *);
void init_hybrid_table(bp_hybrid *);
void init_predictor(bp_params *, int, int);
void init_hybrid_predictor(bp_hybrid * , int, int, int, int);
void bimodal_prediction(bp_params *, unsigned int, const char);
void gshare_prediction(bp_params *, unsigned int, const char);
void hybrid_prediction(bp_hybrid *, unsigned int, const char);
void print_order(bp_params *, char);
void hybrid_print_order(bp_hybrid *);

#endif
