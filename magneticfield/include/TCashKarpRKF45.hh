//template mode of RKF45
#include "G4LineSection.hh"
#include "G4CashKarpRKF45.hh"

template
<class T_Equation, class T_Field>
class TCashKarpRKF45 : public G4CashKarpRKF45
{
	public:
		TCashKarpRKF45(T_Equation *EqRhs,
				G4int noIntegrationVariables=6, G4bool primary=true)
			: G4CashKarpRKF45(EqRhs),
			fEquation_Rhs(EqRhs)
	{
		const G4int numberOfVariables = noIntegrationVariables;

		ak2 = new G4double[numberOfVariables] ;  
		ak3 = new G4double[numberOfVariables] ; 
		ak4 = new G4double[numberOfVariables] ; 
		ak5 = new G4double[numberOfVariables] ; 
		ak6 = new G4double[numberOfVariables] ; 
		ak7 = 0;
		yTemp = new G4double[numberOfVariables] ; 
		yIn = new G4double[numberOfVariables] ;
		fLastInitialVector = new G4double[numberOfVariables] ;
		fLastFinalVector = new G4double[numberOfVariables] ;
		fLastDyDx = new G4double[numberOfVariables];

		fMidVector = new G4double[numberOfVariables];
		fMidError =  new G4double[numberOfVariables];
		if( primary )
		{ 
			fAuxStepper = new TCashKarpRKF45(EqRhs, numberOfVariables, !primary);
		}
	}

		~TCashKarpRKF45()
		{
			delete[] ak2;
			delete[] ak3;
			delete[] ak4;
			delete[] ak5;
			delete[] ak6;
			// delete[] ak7;
			delete[] yTemp;
			delete[] yIn;
			delete[] fLastInitialVector;
			delete[] fLastFinalVector;
			delete[] fLastDyDx;
			delete[] fMidVector;
			delete[] fMidError; 
		}

		inline void TRightHandSide(G4double y[], G4double dydx[]) 
		{fEquation_Rhs->TRightHandSide(y, dydx);}

		void Stepper(const G4double yInput[],
				const G4double dydx[],
				G4double Step,
				G4double yOut[],
				G4double yErr[])
		{
			// const G4int nvar = 6 ;
			// const G4double a2 = 0.2 , a3 = 0.3 , a4 = 0.6 , a5 = 1.0 , a6 = 0.875;
			G4int i;

			const G4double  b21 = 0.2 ,
				  b31 = 3.0/40.0 , b32 = 9.0/40.0 ,
				  b41 = 0.3 , b42 = -0.9 , b43 = 1.2 ,

				  b51 = -11.0/54.0 , b52 = 2.5 , b53 = -70.0/27.0 ,
				  b54 = 35.0/27.0 ,

				  b61 = 1631.0/55296.0 , b62 =   175.0/512.0 ,
				  b63 =  575.0/13824.0 , b64 = 44275.0/110592.0 ,
				  b65 =  253.0/4096.0 ,

				  c1 = 37.0/378.0 , c3 = 250.0/621.0 , c4 = 125.0/594.0 ,
				  c6 = 512.0/1771.0 ,
				  dc5 = -277.0/14336.0 ;

			const G4double dc1 = c1 - 2825.0/27648.0 ,  dc3 = c3 - 18575.0/48384.0 ,
				  dc4 = c4 - 13525.0/55296.0 , dc6 = c6 - 0.25 ;

			// Initialise time to t0, needed when it is not updated by the integration.
			//        [ Note: Only for time dependent fields (usually electric) 
			//                  is it neccessary to integrate the time.] 
			yOut[7] = yTemp[7]   = yIn[7]; 

			const G4int numberOfVariables= this->GetNumberOfVariables(); 
			// The number of variables to be integrated over

			//  Saving yInput because yInput and yOut can be aliases for same array

			for(i=0;i<numberOfVariables;i++) 
			{
				yIn[i]=yInput[i];
			}
			// RightHandSide(yIn, dydx) ;              // 1st Step

			for(i=0;i<numberOfVariables;i++) 
			{
				yTemp[i] = yIn[i] + b21*Step*dydx[i] ;
			}
			TRightHandSide(yTemp, ak2) ;              // 2nd Step

			for(i=0;i<numberOfVariables;i++)
			{
				yTemp[i] = yIn[i] + Step*(b31*dydx[i] + b32*ak2[i]) ;
			}
			TRightHandSide(yTemp, ak3) ;              // 3rd Step

			for(i=0;i<numberOfVariables;i++)
			{
				yTemp[i] = yIn[i] + Step*(b41*dydx[i] + b42*ak2[i] + b43*ak3[i]) ;
			}
			TRightHandSide(yTemp, ak4) ;              // 4th Step

			for(i=0;i<numberOfVariables;i++)
			{
				yTemp[i] = yIn[i] + Step*(b51*dydx[i] + b52*ak2[i] + b53*ak3[i] +
						b54*ak4[i]) ;
			}
			TRightHandSide(yTemp, ak5) ;              // 5th Step

			for(i=0;i<numberOfVariables;i++)
			{
				yTemp[i] = yIn[i] + Step*(b61*dydx[i] + b62*ak2[i] + b63*ak3[i] +
						b64*ak4[i] + b65*ak5[i]) ;
			}
			TRightHandSide(yTemp, ak6) ;              // 6th Step

			for(i=0;i<numberOfVariables;i++)
			{
				// Accumulate increments with proper weights

				yOut[i] = yIn[i] + Step*(c1*dydx[i] + c3*ak3[i] + c4*ak4[i] + c6*ak6[i]) ;

				// Estimate error as difference between 4th and
				// 5th order methods

				yErr[i] = Step*(dc1*dydx[i] + dc3*ak3[i] + dc4*ak4[i] +
						dc5*ak5[i] + dc6*ak6[i]) ;

				// Store Input and Final values, for possible use in calculating chord
				fLastInitialVector[i] = yIn[i] ;
				fLastFinalVector[i]   = yOut[i];
				fLastDyDx[i]          = dydx[i];
			}
			// NormaliseTangentVector( yOut ); // Not wanted

			fLastStepLength =Step;

			return ;
		}


		G4double  DistChord()   const
		{
			G4double distLine, distChord; 
			G4ThreeVector initialPoint, finalPoint, midPoint;

			// Store last initial and final points (they will be overwritten in self-Stepper call!)
			initialPoint = G4ThreeVector( fLastInitialVector[0], 
					fLastInitialVector[1], fLastInitialVector[2]); 
			finalPoint   = G4ThreeVector( fLastFinalVector[0],  
					fLastFinalVector[1],  fLastFinalVector[2]); 

			// Do half a step using StepNoErr

			fAuxStepper->Stepper( fLastInitialVector, fLastDyDx, 0.5 * fLastStepLength, fMidVector,   fMidError );

			midPoint = G4ThreeVector( fMidVector[0], fMidVector[1], fMidVector[2]);       

			// Use stored values of Initial and Endpoint + new Midpoint to evaluate
			//  distance of Chord


			if (initialPoint != finalPoint) 
			{
				distLine  = G4LineSection::Distline( midPoint, initialPoint, finalPoint );
				distChord = distLine;
			}
			else
			{
				distChord = (midPoint-initialPoint).mag();
			}
			return distChord;
		}

	private:
		G4double *ak2, *ak3, *ak4, *ak5, *ak6, *ak7, *yTemp, *yIn;
		// scratch space

		G4double fLastStepLength;
		G4double *fLastInitialVector, *fLastFinalVector,
				 *fLastDyDx, *fMidVector, *fMidError;
		// for DistChord calculations

		TCashKarpRKF45* fAuxStepper; 
		T_Equation* fEquation_Rhs;
};

