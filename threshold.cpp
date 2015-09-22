// --*- C++ -*--
/*
 *  iirlib Threshold Module
 *
 *	threshold.cpp
 *	Last modifyed on 2008-09-30	by Tetsunari Inamura
 *
 *	Copyright (c) Tetsunari Inamura 1998--2015
 *	All Rights Reserved.
 */

/* Change Log
 *
 * 1998 Dec 10th
 *	created from another project
 * 1998 Dec 16th
 *	Threshold_InitLever debugged
 *	Threshold_SetValue  debugged
 * 1999 Jan 6th
 *	target of threshold changed from int to double
 * 1998 Feb 17th
 *	separeted from another project (database.c)
 * 1999 Mar 8th
 *	LoadFromFile added
 *	2nd dimension is added
 * 1999 Mar 13th
 *	several debugging
 * 1999 Jun 22nd
 *	if VALUE_UNOBSERVED is found, discrete process will skip it
 * 1999 Jul 12th
 *	Threshold::AddNewState added
 *	Threshold::SaveToFile added
 * 1999 Jul 22nd
 *	mean value added
 * 1999 Oct 15th
 *	member variable 'mean' added
 * 2008 Jan 29th
 *	change into c++ by Tareeq
 * 2008 Feb 9th
 *	TL_SUCCESS, TL_FAIL discarded. Using TRUE and FALSE
 * 2008 Feb 17th
 *	debug flag is embedded in class
 * 2008 Sep 27th
 *	Modify InsertSegment , add PushBackSegment
 */


/* TODO
 *	It will be required to check the number of lines describing "Number of state" in Constructor
 */


#include "threshold.h"
#include "iirlib.h"



Threshold::Threshold ()
{
	debug=0;
	segment_seq.resize(0);
	if (debug) tl_message ("threshold is created");
}



/*----------------------------------------------------------------------------*/
// Added : 1999 Mar 8th
// Memo : Set the MIN and MAX value
// Input : target_index	: index for discrete segment
// Input : max_v
// Input : min_v
// Input : state_value	: discrete value of the target segment (normally, it is equal to the target_index)
/*----------------------------------------------------------------------------*/
int Threshold::SetValue (int target_index, double max_v, double min_v, int discrete_state_value)
{
	if (target_index<0 || target_index > number_of_state)
		{
			tl_warning ("No such state No.%d", target_index);
			return FALSE;
		}
	segment_seq[target_index].SetMinMaxValue (0, min_v, max_v);
	segment_seq[target_index].DiscreteValue (discrete_state_value);

	return TRUE;
}


int Threshold::SetDebug (int value)
{
	debug = value;
	return TRUE;
}


/*----------------------------------------------------------------------------*/
// Added    : 1999 Mar 8th
// Debugged : 1999 Mar 13th : change the order of 2D array like a max[][] (inamura misunderstood the order)
// Modified : 1999 Jul 22nd : Name registor function
// Input : *filename		: Threshold type file "*.thd" (FullPath is required)
/*----------------------------------------------------------------------------*/
int Threshold::LoadFromFile (char *filename)
{
	FILE    *fp;
	char    line[MAX_STRING], *charp;
	int     i, j, tmp, state_no, min, max, mean, ret;

	if (debug) tl_message ("File : %s", filename);
	// if the suffix is not ".thd", it becomes to be error
	if (!tl_strmember (filename, ".thd")) {
		tl_warning ("This file {%s} doesn't include .thd suffix!", filename);
		return FALSE;
	}
	// Cehck the existance of the file
	if (( fp = fopen( filename, "r"))==NULL) {
		tl_warning ("No such file {%s}", filename);
		// If the previous file is not 'free-ed', the return might be NULL
		return FALSE;
	}

	label = filename;
	fgets (line, MAX_STRING, fp);
	if (!tl_strmember( line, "# Number of State")) {
		tl_message ("syntax error in %s, Description Error [%s]", filename, line);
		return FALSE;
	}
	fgets (line, MAX_STRING, fp);
	sscanf( line, "%d", &tmp );
	if (tmp <= 0) {
		tl_warning ("Syntax error: number of State = %d", tmp);
		return FALSE;
	}
	number_of_state = tmp;
	segment_seq.resize(number_of_state);
  
	// Reading the number of dimension of raw data
	fgets (line, MAX_STRING, fp);
	if (!tl_strmember( line, "Parameter Dimension")) {
		tl_message ("Description Error %s", line );
		return FALSE;
	}
	fgets (line, MAX_STRING, fp);
	sscanf (line, "%d", &tmp);
	if (tmp <= 0) {
		tl_warning ("Syntax Error: Dimension = %d", tmp);
		return FALSE;
	}
	dimension = tmp;

	// reading each line after skipping one line
	fgets (line, MAX_STRING, fp);
	for (i=0; i<number_of_state; i++)
		{
			// loop for number of states
			fgets (line, MAX_STRING, fp);
			charp = line;
			// Reading index of the target segment
			ret = sscanf_int_with_seeking (&charp, &state_no);
			if (state_no < 0)
				{
					tl_warning ("State No. is wrong %d", state_no);
					return FALSE;
				}
			// Reading value for the segment
			ret = sscanf_int_with_seeking( &charp, &tmp);
			if (tmp < 0)
				{
					tl_warning ("Discrete Value is wrong %d", tmp);
					return FALSE;
				}
			segment_seq[state_no].DiscreteValue (tmp);

			// Reading Min and Max value for j-th dimension of the segment
			for (j=0; j<dimension; j++)
				{
					ret = sscanf_int_with_seeking (&charp, &min);
					if (debug) tl_message("read min value = %d", min);
					ret = sscanf_int_with_seeking (&charp, &max);
					if (debug) tl_message("read max value = %d", max);
					if (min > max)
						{
							tl_warning ("min greater than max : min = %d, max = %d!!, in file %s", min, max, filename);
							tl_warning ("State No.%d State Value=%d", state_no, segment_seq[state_no].DiscreteValue() );
							return -1;
						}
					if (debug) tl_message ("State No.%d , dim = %d : max = %d , min = %d", state_no, j, max, min );
					segment_seq[state_no].SetMinMaxValue (j, min, max);  // for j-th dimension
					//max[j][state_no] = max;
					//min[j][state_no] = min;
					// If mean value is not described, (min+max)/2 is automatically set (added on 1999 Oct 15th)
					ret = sscanf_int_with_seeking (&charp, &mean);
					if (debug) tl_message ("sscanf_int_with_seeking done");
					if (ret==FALSE)
						{
							if (debug) tl_message ("mean not found (state_no = %d, j = %d)", state_no, j);
							segment_seq[state_no].SetMeanValue (j, (max + min)/2);
							if (debug) tl_message("there is no mean value, so set %d", (int)(max + min)/2);
						}
					else
						{
							if (debug) tl_message ("mean found");
							segment_seq[state_no].SetMeanValue (j, mean);
							if (debug) tl_message("read and set mean value = %d", mean);
						}
				}
		}
	fclose (fp);
	if (debug) tl_message ("finished successfully");
	return 1;
}



/*----------------------------------------------------------------------------*/
// Added : 1999 Mar 8th
/*----------------------------------------------------------------------------*/
Threshold::~Threshold ()
{
	if (debug) Verify();
#if 0
	for (vector<SegmentState>::iterator p=segment_seq.begin(); p!=segment_seq.end(); p++) {
		delete *p;
	}
#endif
}


/*----------------------------------------------------------------------------*/
// Added : 1999 Jul 12th
// Memo  : reference of member variable number_of_state
/*----------------------------------------------------------------------------*/
int Threshold::NumOfState (void)
{
	return number_of_state;
}


/*----------------------------------------------------------------------------*/
// Added : 1999 Jul 12th
// return : (int)		: dimension
/*----------------------------------------------------------------------------*/
int Threshold::Dimension (void)
{
	return dimension;
}

/*----------------------------------------------------------------------------*/
// Created : 2008-09-30 by inamura
// input   : (int)		: dimension
/*----------------------------------------------------------------------------*/
int Threshold::Dimension (int dim)
{
	dimension = dim;
	return dimension;
}

/*----------------------------------------------------------------------------*/
// Created on 2008-01-23
/*----------------------------------------------------------------------------*/
int Threshold::MaxDiscreteValue(void)
{
	int value = 0;
	for (int i=0; i<number_of_state; i++) {
		if (segment_seq[i].DiscreteValue() > value)
			value = segment_seq[i].DiscreteValue();
	}
	return value;
}


// Created on 2008-01-23
int Threshold::SortDiscreteValue(void)
{
	for (int i=0; i<number_of_state; i++) {
		segment_seq[i].DiscreteValue(i);
	}
	return TRUE;
}


/*----------------------------------------------------------------------------*/
// Added : 1999 Jul 12th
// Memo : Add a new SegmentState
// Input : *max		: max value of the new segment
// Input : *min		: min value of the new segment
/*----------------------------------------------------------------------------*/
int Threshold::AddNewState (double min_value, double max_value)
{
	SegmentState	segment (min_value, max_value);
	if (debug) tl_message ("Start!");

	// The new value of state is one more larger than the current max value
	segment.DiscreteValue (MaxDiscreteValue()+1);
	number_of_state++;
	segment_seq.push_back (segment);

	if (debug) tl_message ("End!");
	return 1;
}


#if 0
/*----------------------------------------------------------------------------*/
// Added   : 1999 Oct 17th
/*----------------------------------------------------------------------------*/
int Threshold::AddDiscreteState (int value)
{
	number_of_state++;

	if (debug) tl_message ("add value %d", value);

	return 1;
}
#endif


/*----------------------------------------------------------------------------*/
// Added    : 1999 Jul 12th
/*----------------------------------------------------------------------------*/
int Threshold::SaveToFile (char *filename)
{
	int		i, j;
	FILE		*fp;

	if (debug) tl_message ("Start");
	if ((fp = fopen(filename, "w"))==NULL)
		{
			tl_warning ("Cannot open the file : %s", filename);
			return FALSE;
		}
	if (debug) tl_message ("Filename : %s", filename);
	fprintf( fp, "# Number of State\n%d\n",     number_of_state );
	fprintf( fp, "# Parameter Dimension\n%d\n", dimension );
	if (dimension==2 )
		fprintf( fp, "# State No.&Value       min  max mean\tmin  max mean\n");
	else
		fprintf( fp, "# State No.&Value       min  max mean\n");

	for (i=0; i<number_of_state; i++)
		{
			fprintf (fp, "%d\t%d\t", i, segment_seq[i].DiscreteValue());
			for (j=0; j<dimension; j++)
				{
					fprintf (fp, "%g   %g   %g", segment_seq[i].MinValue(j), segment_seq[i].MaxValue(j), segment_seq[i].Mean(j) );
					if (j!=dimension-1)
						fprintf (fp, "\t");
				}
			fprintf (fp, "\n");
		}
	fclose(fp);
	if (debug) tl_message ("End");
	return TRUE;
}




/*----------------------------------------------------------------------------*/
// Added : 1999 Jul 22nd
// Mem o : query of typical data (mean value)
// Input : state	: discrete value of target state
// Output: *data	: typical data (pointer for the sequence of data)
/*----------------------------------------------------------------------------*/
vector<double> Threshold::TypicalData (int state_val, int *data)
{
	for (int i=0; i<number_of_state; i++)
		{
			if (segment_seq[i].DiscreteValue() == state_val)
				return segment_seq[i].MeanVector();
		}
	tl_warning ("No such state : No.%d in {%s}", state_val, label.c_str());
	return segment_seq[0].MeanVector();
}


/*----------------------------------------------------------------------------*/
// Created  on 2008-01-23 by inamura
// Modified on 2008-09-24 by inamura
/*----------------------------------------------------------------------------*/
int Threshold::DivideState (int index)
{
	SegmentState *new_segment = NULL;
	// min value of No.index    : not changed
	// max value of No.index    : changed to (min + max) / 2 of old No.index
	// min value of No.index+1  : changed to (min + max) / 2 of old No.index
	// max value of No.index+1  : changed to max of old No.index

	for (int dim=0; dim<dimension; dim++) {
		double new_thre_value = (segment_seq[index].MinValue(dim) + segment_seq[index].MaxValue(dim)) / 2;
		if (debug) tl_message ("new_thre_value = %g (%g + %g)/2", new_thre_value, segment_seq[index].MinValue(dim), segment_seq[index].MaxValue(dim));
		new_segment = new SegmentState (new_thre_value, segment_seq[index].MaxValue(dim));
		segment_seq[index].SetMaxValue(dim, new_thre_value);
		InsertSegment (index, new_segment);
	}
	number_of_state++;
	return TRUE;
}


// Created on 2008-01-23 by inamura
int Threshold::ConcatenateState (int low_index, int high_index)
{
	// Low_index's   min    ,   (max)
	// High_index's  (min)  ,   max

	if (debug) tl_message("start");
	for (int dim=0; dim<dimension; dim++) {
		segment_seq[low_index].SetMaxValue(dim, segment_seq[high_index].MaxValue(dim));
		vector<SegmentState>::iterator p = segment_seq.begin();
		p += high_index;
		segment_seq.erase (p);
	}
	number_of_state--;
	SortDiscreteValue();
	return TRUE;
}


/*----------------------------------------------------------------------------*/
// Added : 1999 Mar 13th
// Memo  : Debug message
/*----------------------------------------------------------------------------*/
int Threshold::Verify(void)
{
	int		i, j;

	cerr << "------ Print All ------" << endl;
	cerr << "number_of_state =" << number_of_state << endl;
	cerr << "dimension       =" << dimension << endl;
	for (i=0; i<dimension; i++) {
		for (j=0; j<number_of_state; j++)
			segment_seq[j].Verify();
	}
	cerr << endl;
	tl_message ("------ Print All End ------ \n\n");
	return TRUE;
}



/*----------------------------------------------------------------------------*/
// Modified : 1999 Jun 22nd
// Input : double value		: Target data to be discreted
// Return : (int)		: If the input data was unobserved data (-9999), it will be returned without midification
// Memo : This function is valid for only 1-Dimensional value
/*----------------------------------------------------------------------------*/
int Threshold::Query1D (double value)
{
	int		i;

	if (debug) tl_message ("thre=%s  ,  value = %g", label.c_str(), value );
	for (i=0; i<number_of_state; i++)
		if (segment_seq[i].MaxValue(0) >= value && segment_seq[i].MinValue(0) <= value) {
			if (debug) tl_message ("result = %d", i);
			return segment_seq[i].DiscreteValue();
		}

	if (value==VALUE_UNOBSERVED)
		return VALUE_UNOBSERVED;
	tl_warning ("No such category (value = %g) in {%s}", value, label.c_str());
	return -1;
}



/*----------------------------------------------------------------------------*/
// Added : 1999 Jan 6th
// Memo : Discrete 2 dimensional data
/*----------------------------------------------------------------------------*/
int Threshold::Query2D (double value1, double value2)
{
	int		i;

	for (i=0; i<number_of_state; i++)
		{
			SegmentState  seg = segment_seq[i];
			if (seg.MaxValue(0) >= value1 && seg.MinValue(0) <= value1 && seg.MaxValue(1) >= value2 && seg.MinValue(1) <=value2)
				{
					if (debug) tl_message ("(%g, %g) --> %d", value1, value2, i);
					return seg.DiscreteValue();
				}
		}

	if (value1==VALUE_UNOBSERVED)
		return VALUE_UNOBSERVED;
	tl_warning ("No such category (value = %g, %g) in {%s}", value1, value2, label.c_str());
	return FALSE;
}



/*----------------------------------------------------------------------------*/
// Added    : 1999 Mar 8th
// Modified : 2008 Feb 9th : Integrate of 1-D and 2-D Querying
// Input    : double value : value to be discrete
// Return   : (int)	   : Discrete value
/*----------------------------------------------------------------------------*/
int Threshold::QueryDiscreteValue (double continuous_value)
{
	if (dimension!=1) {
		tl_warning ("dimension is not 1");
		return FALSE;
	}
	return Query1D (continuous_value);
}


/*----------------------------------------------------------------------------*/
// Added    : 1999 Mar 8th
// Modified : 2008 Feb 9th : Integrate of 1-D and 2-D Querying
// Return   : (int)	   : Discrete value
/*----------------------------------------------------------------------------*/
int Threshold::QueryDiscreteValue (vector<double> vec)
{
	if (vec.size() == 1 || vec.size() > 2) {
		tl_warning ("dimension is not 2");
		return FALSE;
	}
  
	return Query2D (vec[0], vec[1]);
}



/*----------------------------------------------------------------------------*/
// Memo : Changing the joystick lever input into discrete value
// Input : x		: x coordinate of the joystick lever (Left:Right <--> -100:+100)
// Input : y		: y coordinate of the joystick lever (Left:Right <--> -100:+100)
/*----------------------------------------------------------------------------*/
int Threshold::LeverXYToState (int x, int y)
{
	double	rad;
	int		i, deg, scale;
  
	rad = atan2( (double)y, (double)x );
	if (rad < 0)
		deg = (int) ( (rad + 2*M_PI) * 180 / M_PI );
	else
		deg = (int) ( rad * 180 / M_PI );

	scale = (int)sqrt( (double)x*x + (double)y*y );

  
	for (i=0; i<number_of_state; i++)
		{
			SegmentState seg = segment_seq[i];
			if (seg.MaxValue(0) >= deg && seg.MinValue(0) <= deg && seg.MaxValue(1) >= scale && seg.MaxValue(1) <= scale)
				return seg.DiscreteValue();
		}
	tl_message ("!Error! (x,y)=(%d,%d)  :: (deg,scl)=(%d,%d)", x, y, deg, scale);
	return FALSE;
}



int Threshold::InsertSegment (int index, SegmentState *segment)
{
	int	debug=0;

	if (debug) tl_message ("start");
	if (debug) Verify();
	vector<SegmentState>::iterator p;
	number_of_state ++;
	p = segment_seq.begin();
	p += index+1;
	segment_seq.insert (p, *segment);
	if (debug) tl_message ("end");
	if (debug) Verify();
	return TRUE;
}


/*----------------------------------------------------------------------------*/
// Created  on 2008-09-24 by inamura
/*----------------------------------------------------------------------------*/
int Threshold::PushBackSegment (SegmentState *segment)
{
	number_of_state ++;
	segment_seq.push_back (*segment);
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
//                                     SegmentState                                           //
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

SegmentState::SegmentState ()
{
	debug=0;
	tl_message ("segment state is created");
	min.resize(1);
	max.resize(1);
	mean.resize(1);
}


SegmentState::SegmentState (double min_value, double max_value)
{
	min.resize(1);
	max.resize(1);
	mean.resize(1);
	min[0] = min_value;
	max[0] = max_value;
	mean[0] = (double)(min_value + max_value)/2;
}


SegmentState::~SegmentState ()
{
	//tl_message ("segment state is deleted");
}


double SegmentState::MinValue (int dim)
{
	return min[dim];
}

int SegmentState::SetMinValue (int dim, double min_value)
{
	max[dim] = min_value;
	return TRUE;
}

double SegmentState::MaxValue (int dim)
{
	return max[dim];
}

int SegmentState::SetMaxValue (int dim, double max_value)
{
	max[dim] = max_value;
	return TRUE;
}

double SegmentState::Mean (int dim)
{
	return mean[dim];
}

vector<double> SegmentState::MeanVector (void)
{
	return mean;
}

int SegmentState::Verify (void)
{
	cerr << "Max = " << max[0] << "\tMin = " << min[0] << "\tDiscreteValue = " << discrete_value << endl;
	return 1;
}

int SegmentState::SetMinMaxValue (int dim, double min_value, double max_value)
{
	if (dim<0 || dim>=(int)(max.size()))
		{
			tl_warning ("input dimension <%d> is wrong parameter", dim);
			return FALSE;
		}
	else if (1)
		{
			tl_message ("set min=%g, max=%g as %d-th dimension", min_value, max_value, dim);
		}
	min[dim] = min_value;
	max[dim] = max_value;
	tl_message ("end of function");
	return 1;
}

int SegmentState::SetMeanValue (int dim, double mean_value)
{
	mean[dim] = mean_value;

	return 1;
}

int SegmentState::DiscreteValue (int value)
{
	discrete_value = value;

	return 1;
}

int SegmentState::DiscreteValue (void)
{
	return discrete_value;
}


/*----------------------------------------------------------------------------*/
// Memo : Set the MAX and MIN values for 2-D threshold
// Input : state_value		: Discrete value for the segment
// Input : min_p1		: Min value for 1st dimension
// Input : max_p1		: Max value for 1st dimension
// Input : min_p2		: Min value for 2nd dimension
// Input : max_p2		: Max value for 2nd dimension
/*----------------------------------------------------------------------------*/
int SegmentState::SetValue2D (int state_val, double min_p1, double max_p1, double min_p2, double max_p2)
{
	max[0] = max_p1;
	min[0] = min_p1;
	max[1] = max_p2;
	min[1] = min_p2;
	discrete_value = state_val;

	return TRUE;
}

