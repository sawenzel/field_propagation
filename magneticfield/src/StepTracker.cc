/*
 * StepTracker.cc
 *
 *  Created on: Jul 12, 2015
 *      Author: jason
 */

#include <StepTracker.hh>

#include <iostream>

#include <vector>
using namespace std;

#define BUFFER_COLUMN_LEN 10
#define TIME_SLOT 7

StepTracker::StepTracker() {

   buffer_ptr = new vector< vector<G4double> > ();
   //cout << "allocated" << endl;
}

StepTracker::~StepTracker() {

   delete buffer_ptr;
}


void StepTracker::outputBuffer() {

   vector< vector<G4double> > &buffer = *buffer_ptr;

   for (int i = 0; i < getBufferLength(); i ++) {
      for (int j = 0; j < BUFFER_COLUMN_LEN; j ++)
         cout << buffer[i][j] << ",";
      cout << endl;
   }

}

void StepTracker::ReportCurveLength(G4double current_curve_length, G4double htry) {
   cout << "ReportCurveLength: " << current_curve_length
         << " htry: " << htry << endl;

}


void StepTracker::RecordResultOfStepper( G4double yIn[], G4double dydx[]) {

   vector< vector<G4double> > &buffer = *buffer_ptr;

   // Only record data from the beginning of the interval.
   // This is because there is no pre-set way to pass the last RHS
   // evaluation to StepTracker from the BaseStepper of MagIntegratorStepper_byTime. (Yet.)

   buffer_ptr -> push_back( vector<G4double> (BUFFER_COLUMN_LEN) );
   G4int last_index = buffer_ptr -> size() - 1;
   buffer[last_index][0] = yIn[TIME_SLOT];
   for (int i = 0; i < 6; i ++) {
      buffer[last_index][i + 1] = yIn[i];
   }
   for (int i = 3; i < 6; i ++) {
      buffer[last_index][i + 3] = dydx[i];
   }
   /*
   cout << "yIn: ";
   for (int i = 0; i < BUFFER_COLUMN_LEN; i ++) {
      if (i < 3)
         cout << yIn[i] << "," ;
      last_y[i] = yIn[i];
   }
   cout << endl;
   */
}

void StepTracker::StepsAccepted( G4double newCurveLength ) {

   /*
   vector< vector<G4double> > &buffer = *buffer_ptr;

   buffer_ptr -> push_back( vector<G4double> (BUFFER_COLUMN_LEN) );
   G4int last_index = buffer_ptr -> size() - 1;
   for (int i = 0; i < 3; i ++) {
      buffer[last_index][i] = last_y[i];
   }
   buffer[last_index][TIME_SLOT] = newCurveLength;




   cout << "StepAccepted, New Curve Length: " << newCurveLength << endl;
   */
}