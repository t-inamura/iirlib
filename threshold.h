// --*- C++ -*--
/*
 *  iirlib Threshold Module
 *
 *	threshold.h
 *	Last modifyed on 2008-09-30 by Tetsunari Inamura
 *
 *	Copyright (c) Tetsunari Inamura 1998--2015.
 *	All Rights Reserved.
 */


#ifndef __THRESHOLD_H__
#define __THRESHOLD_H__

#define VALUE_UNOBSERVED    -9999

#include <string>
#include <vector>

using namespace std;


class SegmentState
{
	int             index;
	int             discrete_value;
	//double        state_value;		// Value for each state (not index no.). The same value is allowed for different states
	vector<double>  max;
	vector<double>  min;
	vector<double>  mean;
	int             debug;

public:
	SegmentState          ();
	SegmentState          (double min_value, double max_value);
	~SegmentState         ();
	int    SetMinMaxValue (int dim, double min_value, double max_value);
	int    SetMeanValue   (int dim, double mean_value);
	int    DiscreteValue  (int value);
	int    DiscreteValue  (void);
	double MinValue       (int dim);
	double MaxValue       (int dim);
	double Mean           (int dim);
	int    SetMinValue    (int dim, double min_value);
	int    SetMaxValue    (int dim, double max_value);
	int    Verify         (void);
	int    SetValue2D     (int state_val, double min_p1, double max_p1, double min_p2, double max_p2);
	vector<double> MeanVector (void);
};


// Created  on 1999 Mar 8th
// Modified on 2008-01-22 by inamura
class Threshold
{
	string	label;
	int		number_of_state;	// No. of states
	int		dimension;		// No. of dimension of raw data
	vector<SegmentState>	segment_seq;
	int		debug;

public:
	Threshold ();
	~Threshold ();
	int            LoadFromFile       (char *filename);
	int            NumOfState         (void);
	int            Dimension          (void);
	int            Dimension          (int dim);
	int            AddNewState        (double min_value, double max_value);
	int            MaxDiscreteValue   (void);
	int            SortDiscreteValue  (void);
	int            SetValue           (int state, double max_v, double min_v, int discrete_state_value);
	int            SetDebug           (int value);
	int            AddDiscreteState   (int value);
	int            SaveToFile         (char *filename);
	vector<double> TypicalData        (int state, int *data);
	int            Query1D            (double value);
	int            Query2D            (double value1, double value2);
	//int          DiscreteValue      (double continuous_value);
	int            QueryDiscreteValue (double value);
	int            QueryDiscreteValue (vector<double> vec);
	int            LeverXYToState     (int x, int y);
	int            DivideState        (int index);
	int            Verify             (void);
	int            InsertSegment      (int index, SegmentState *segment);
	int            PushBackSegment    (SegmentState *segment);
	int            ConcatenateState   (int low_index, int high_index);
};

#endif  /*__THRESHOLD_H__*/
