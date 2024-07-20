#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <math.h>
#include "sim_bp.h"
using namespace std;

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    
    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);

        
        // Open trace_file in read mode
        FP = fopen(trace_file, "r");
        if(FP == NULL)
        {
            // Throw error and exit if fopen() failed
            printf("Error: Unable to open file %s\n", trace_file);
            exit(EXIT_FAILURE);
        }

        // Initialize the prediction table 
        bimodal = (struct bp_params *)calloc(1, sizeof(bp_params));
		init_predictor(bimodal, params.M2, 0); // Set N=0 since only M is inputted

        char str[2];
        while(fscanf(FP, "%lx %s", &addr, str) != EOF)
        {
            outcome = str[0];
            bimodal_prediction(bimodal, addr, outcome);
        }

        print_order(bimodal, 'b');
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);
        
        // Open trace_file in read mode
        FP = fopen(trace_file, "r");
        if(FP == NULL)
        {
            // Throw error and exit if fopen() failed
            printf("Error: Unable to open file %s\n", trace_file);
            exit(EXIT_FAILURE);
        }

        // Initialize the prediction table 
        gshare = (struct bp_params *)calloc(1, sizeof(bp_params));
		init_predictor(gshare, params.M1, params.N);

        char str[2];
        while(fscanf(FP, "%lx %s", &addr, str) != EOF)
        {
            outcome = str[0];
            if (params.N == 0) // If N=0, it's actually a bimodal
            {
                bimodal_prediction(gshare, addr, outcome);
            }
            else
            {
                gshare_prediction(gshare, addr, outcome);
            }
        }

        print_order(gshare, 'g');
    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);
        
        // Open trace_file in read mode
        FP = fopen(trace_file, "r");
        if(FP == NULL)
        {
            // Throw error and exit if fopen() failed
            printf("Error: Unable to open file %s\n", trace_file);
            exit(EXIT_FAILURE);
        }

        // Initialize the prediction table 
        hybrid = (struct bp_hybrid *)calloc(1, sizeof(bp_hybrid));
		init_hybrid_predictor(hybrid, params.K, params.M1, params.N, params.M2);

        char str[2];
        while(fscanf(FP, "%lx %s", &addr, str) != EOF)
        {
            outcome = str[0];
            hybrid_prediction(hybrid, addr, outcome);
        }

        hybrid_print_order(hybrid);
    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    
    // char str[2];
    // while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    // {
        
    //     outcome = str[0];
    //     if (outcome == 't')
    //         printf("%lx %s\n", addr, "t");           // Print and test if file is read correctly
    //     else if (outcome == 'n')
    //         printf("%lx %s\n", addr, "n");          // Print and test if file is read correctly
    //     /*************************************
    //         Add branch predictor code here
    //     **************************************/
    // }
    return 0;
}


void init_table(bp_params *bp)
{
    // Initialize all entries in the prediction table to 2
	for (int i = 0; i < bp->table_rows; i++)
	{
		bp->table[i] = 2;
	}

	// Initialize branch history register to 0
	bp->BHR_table = 0;
}

void init_hybrid_table(bp_hybrid *bp)
{
    // Initialize all entries in the prediction table to 2
	for (int i = 0; i < bp->Btable_rows; i++)
	{
		bp->table_b[i] = 2;
	}

	for (int j = 0; j < bp->Gtable_rows; j++)
	{
		bp->table_g[j] = 2;
	}

    // Initialize counters in the chooser table to 1
	for (int k = 0; k < bp->Htable_rows; k++)
	{
		bp->table_h[k] = 1;
	}

	// Initialize branch history register to 0
	bp->BHR_table = 0;
}

void init_predictor(bp_params *bp, int M, int N)
{
	bp->PC_bits = M;
	bp->BHR_bits = N;

    // Compute rows of table 2^n
	bp->table_rows = pow(2, bp->PC_bits);
	bp->table = new int[bp->table_rows];  // Construct prediction table

    // Compute mask
	bp->PC_b = pow(2, bp->PC_bits)-1; // Bimodal
	bp->PC_g = pow(2, (bp->PC_bits - bp->BHR_bits))-1; // Gshare M-N
	bp->BHR_g = pow(2, bp->BHR_bits)-1; // Gshare

    bp->predictions = 0;
	bp->mispredictions = 0;

    init_table(bp);
}

void init_hybrid_predictor(bp_hybrid *bp , int K, int M1, int N, int M2)
{
    bp->chooser_bits = K;
	bp->PC_bits_g = M1;
	bp->BHR_bits = N;
	bp->PC_bits_b = M2;

    // Compute rows of tables 2^n
	bp->Btable_rows = pow(2, bp->PC_bits_b);
	bp->Gtable_rows = pow(2, bp->PC_bits_g);
	bp->Htable_rows = pow(2, bp->chooser_bits);

    // Construct prediction tables for bimodal, gshare, and hybrid
	bp->table_b = new int[bp->Btable_rows];
	bp->table_g = new int[bp->Gtable_rows];
	bp->table_h = new int[bp->Htable_rows];

	// Compute mask
	bp->PC_b = pow(2, bp->PC_bits_b)-1; // Bimodal
	bp->PC_g = pow(2, (bp->PC_bits_g - bp->BHR_bits))-1; // Gshare M-N
	bp->BHR_g = pow(2, bp->BHR_bits)-1; // Gshare
	bp->PC_h = pow(2, bp->chooser_bits)-1; // Hybrid

    bp->predictions = 0;
    bp->mispredictions = 0;

	init_hybrid_table(bp);
}

void bimodal_prediction(bp_params *bp, unsigned int addr, const char outcome)
{
	bp->predictions++;

	int prediction;
	int actual=-1;
    // Determine the branch’s index into the prediction table.
	bp->index = (addr >> 2) & bp->PC_b;

	// Use index to get the branch’s counter from the prediction table. 
    // If the counter value is greater than or equal to 2, then the branch is predicted taken, else it is predicted not-taken.
	if (bp->table[bp->index] >= 2) 
    {
		prediction = 1;
    }
	else if (bp->table[bp->index] < 2)
    {
		prediction = 0;
    }

	// Get actual outcome
	if (outcome == 'n')
    {
		actual = 0;
    }
	else if (outcome == 't') {
		actual = 1;
    }
	if (prediction != actual)
	{
		bp->mispredictions++;
	}

    // Update the branch predictor based on the branch’s actual outcome. 
    // The branch’s counter in the prediction table is incremented if the branch was taken, decremented if the branch was not-taken.
    // The counter saturates at the extremes (0 and 3), however.
	if (actual == 1)
	{
		bp->table[bp->index] = bp->table[bp->index] + 1;
		if (bp->table[bp->index] > 3) 
        {
			bp->table[bp->index] = 3;
        }
	}
	else if (actual == 0)
	{
		bp->table[bp->index] = bp->table[bp->index] - 1;
		if (bp->table[bp->index] < 0)
        {
			bp->table[bp->index] = 0;
        }
	}

}

void gshare_prediction(bp_params *bp, unsigned int addr, const char outcome)
{
	bp->predictions++;

	int prediction;
	int actual=-1;
	unsigned int temp;

	//cout << hex << addr << '\t' << outcome << endl;
	//cout << "PCmask_gshare " << PCmask_gshare << " Hmask " << Hmask <<  endl;

    // Determine the branch’s index into the prediction table.
	temp = bp->BHR_table ^ ((addr >> (2 + bp->PC_bits - bp->BHR_bits)) & bp->BHR_g);
	bp->index = (temp << (bp->PC_bits - bp->BHR_bits)) + ((addr >> 2) & bp->PC_g);

    // Use index to get the branch’s counter from the prediction table. 
    // If the counter value is greater than or equal to 2, then the branch is predicted taken, else it is predicted not-taken.
	if (bp->table[bp->index] >= 2) 
    {
		prediction = 1;
    }
	else if (bp->table[bp->index] < 2)
    {
		prediction = 0;
    }

    // Get actual outcome
	if (outcome == 'n')
    {
		actual = 0;
    }
	else if (outcome == 't')
    {
		actual = 1;
    }

	if (prediction != actual)
	{
		bp->mispredictions++;
	}

	// Update the branch predictor based on the branch’s actual outcome. 
    // The branch’s counter in the prediction table is incremented if the branch was taken, decremented if the branch was not-taken.
    // The counter saturates at the extremes (0 and 3), however.
	if (actual == 1)
	{
		bp->table[bp->index] = bp->table[bp->index] + 1;
		if (bp->table[bp->index] > 3) 
        {
			bp->table[bp->index] = 3;
        }
	}

	else if (actual == 0)
	{
		bp->table[bp->index] = bp->table[bp->index] - 1;
		if (bp->table[bp->index] < 0) 
        {
			bp->table[bp->index] = 0;
        }
	}

	// Update the global branch history register. 
    // Shift the register right by 1 bit position, and place the branch’s actual outcome into the most-significant bit position of the register.
	bp->BHR_table = (actual << (bp->BHR_bits - 1)) + (bp->BHR_table >> 1);
}

void hybrid_prediction(bp_hybrid *bp , unsigned int addr, const char outcome)
{
	bp->predictions++;

	int actual=-1;
	int prediction_b;
	int prediction_g;
	int prediction_h;
    int match_b;
	int match_g;
	
	unsigned int temp;

    // Get actual outcome
	if (outcome == 'n') 
    {
		actual = 0;
    }
	else if (outcome == 't')
    {
		actual = 1;
    }

    // Obtain two predictions, one from the gshare predictor and one from the bimodal predictor
	// Obtain bimodal prediction
	bp->index_b = (addr >> 2) & bp->PC_b;

	if (bp->table_b[bp->index_b] >= 2) 
    {
		prediction_b = 1;
    }
	else if (bp->table_b[bp->index_b] < 2)
    {
		prediction_b = 0;
    }
	if (prediction_b == actual) 
    {
		match_b = 1;
    }
	else 
    {
        match_b = 0;
    }

	// Obtain gshare prediction
	temp = bp->BHR_table ^ ((addr >> (2 + bp->PC_bits_g - bp->BHR_bits)) & bp->BHR_g);

	bp->index_g = (temp << (bp->PC_bits_g - bp->BHR_bits)) + ((addr >> 2) & bp->PC_g);

	if (bp->table_g[bp->index_g] >= 2)
    {
        prediction_g = 1;
    }
	else if (bp->table_g[bp->index_g] < 2)
    {
		prediction_g = 0;
    }
	if (prediction_g == actual)
    {
	    match_g = 1;
    }
	else 
    {
        match_g = 0;
    }

	// Obtain hybrid prediction
    // Determine the branch’s index into the chooser table.
    // The index for the chooser table is bit k+1 to bit 2 of the branch PC
	bp->index_h = (addr >> 2) & bp->PC_h;

    // Use index to get the branch’s chooser counter from the chooser table.
    // If the chooser counter value is greater than or equal to 2, then use the prediction that was obtained from the gshare predictor, 
    // otherwise use the prediction that was obtained from the bimodal predictor.
	if (bp->table_h[bp->index_h] >= 2) 
    {
		prediction_h = prediction_g;
    }
	else if (bp->table_h[bp->index_h] < 2)
    {
		prediction_h = prediction_b;
    }
	if (prediction_h != actual)
    {
		bp->mispredictions++;
    }

	// Update the selected branch predictor based on the branch’s actual outcome.
    // Only the branch predictor that was selected in step 3, above, is updated.
	if (bp->table_h[bp->index_h] >= 2) // Gshare
	{
		if (actual == 1)
		{
			bp->table_g[bp->index_g] = bp->table_g[bp->index_g] + 1;
			if (bp->table_g[bp->index_g] > 3)
            {
				bp->table_g[bp->index_g] = 3;
            }
		}

		else if (actual == 0)
		{
			bp->table_g[bp->index_g] = bp->table_g[bp->index_g] - 1;
			if (bp->table_g[bp->index_g] < 0)
            {
				bp->table_g[bp->index_g] = 0;
            }
		}

	}
	else if (bp->table_h[bp->index_h] < 2) // Bimodal
	{
		if (actual == 1)
		{
			bp->table_b[bp->index_b] = bp->table_b[bp->index_b] + 1;
			if (bp->table_b[bp->index_b] > 3)
            {
				bp->table_b[bp->index_b] = 3;
            }
		}

		else if (actual == 0)
		{
			bp->table_b[bp->index_b] = bp->table_b[bp->index_b] - 1;
			if (bp->table_b[bp->index_b] < 0)
            {
				bp->table_b[bp->index_b] = 0;
            }
		}

	}

	// Update gshare global branch history register
	bp->BHR_table = (actual << (bp->BHR_bits - 1)) + (bp->BHR_table >> 1);

	// Update branch's chooser counter
	if ((match_g == 1) & (match_b == 0))
	{
		bp->table_h[bp->index_h] = bp->table_h[bp->index_h] + 1;
		if (bp->table_h[bp->index_h] > 3)
        {
			bp->table_h[bp->index_h] = 3;
        }
	}
	else if ((match_g == 0) & (match_b == 1))
	{
		bp->table_h[bp->index_h] = bp->table_h[bp->index_h] - 1;
		if (bp->table_h[bp->index_h] < 0)
        {
			bp->table_h[bp->index_h] = 0;
        }
	}

}

void print_order(bp_params *bp, char mode)
{
	float miss_rate = 100 * (double)(bp->mispredictions) / (double)(bp->predictions);

	cout << "OUTPUT" << endl;
	cout << ' ' << "number of predictions:    " << bp->predictions << endl;
	cout << ' ' << "number of mispredictions: " << bp->mispredictions << endl;
	cout << ' ' << "misprediction rate:       " << setprecision(2) << fixed << miss_rate << "%" << endl;

	if (mode == 'b') {
	    cout << "FINAL BIMODAL CONTENTS" << endl;
    }
	else if (mode == 'g') {
		cout << "FINAL GSHARE CONTENTS" << endl;
    }

	for (int i = 0; i < bp->table_rows; i++)
	{
		cout << ' ' << i << '\t' << bp->table[i] << endl;
	}
}

void hybrid_print_order(bp_hybrid *bp)
{
	float miss_rate = 100 * (double)(bp->mispredictions) / (double)(bp->predictions);

	cout << "OUTPUT" << endl;
	cout << ' ' << "number of predictions:    " << bp->predictions << endl;
	cout << ' ' << "number of mispredictions: " << bp->mispredictions << endl;
	cout << ' ' << "misprediction rate:       " << setprecision(2) << fixed << miss_rate << "%" << endl;
	cout << "FINAL CHOOSER CONTENTS" << endl;

	for (int i = 0; i < bp->Htable_rows; i++)
	{
		cout << i << '\t' << bp->table_h[i] << endl;
	}

	cout << "FINAL GSHARE CONTENTS" << endl;
	for (int i = 0; i < bp->Gtable_rows; i++)
	{
		cout << i << '\t' << bp->table_g[i] << endl;
	}

	cout << "FINAL BIMODAL CONTENTS" << endl;
	for (int i = 0; i < bp->Btable_rows; i++)
	{
		cout << i << '\t' << bp->table_b[i] << endl;
	}
}
