#ifndef _FUNCTION_UTILS_H_
#define _FUNCTION_UTILS_H_
#if defined(_MSC_VER)
#pragma once
#endif

/*
	This software, which is provided in confidence, was prepared by employees
	of Pacific Northwest National Labratory operated by Battelle Memorial
	Institute. Battelle has certain unperfected rights in the software
	which should not be copied or otherwise disseminated outside your
	organization without the express written authorization from Battelle. All rights to
	the software are reserved by Battelle.  Battelle makes no warranty,
	express or implied, and assumes no liability or responisbility for the 
	use of this software.
*/

/*! 
* \file function_utils.h
* \ingroup Objects
* \brief The FunctionUtils class header file.
*
* \author Josh Lurz
* \author Sonny Kim
* \date $Date$
* \version $Revision$
*/

#include <string>
#include <vector>
class Input;
class IFunction;
struct TechChange;

/*! \brief A structure which contains the information neccessary for a production function.
*/
struct ProductionFunctionInfo {
    const std::vector<Input*>& mInputs; //!< The vector of inputs.
    const IFunction* mProductionFunction; //!< Pointer to the technologies production function.
    const double mSigma; //!< The current sigma the production function is using.
    const double mAlphaZeroScaler; //!< Alpha zero used to scale the output of the production.
    const double mCapitalStock; //!< Amount of capital stock the vintage owns.
};

/*! 
* \ingroup Objects
* \brief This class is a set of static helper methods which the various
*        production and demand functions use.
* \details TODO
* \author Sonny Kim, Josh Lurz
*/
class FunctionUtils {
public:
	static void scaleCoefficientInputs( std::vector<Input*>& input,
                                        double scaler );
    
    static void scaleDemandInputs( std::vector<Input*>& input,
                                   double scaler,
                                   const std::string& aRegionName,
                                   const int aPeriod );
    
    static double getDemandSum( const std::vector<Input*>& aInputs );
    
    static double getCoefSum( const std::vector<Input*>& input );
    
    static Input* getInput( const std::vector<Input*>& aInputs,
                            const std::string& aInputName );
    
    static Input* getCapitalInput( const std::vector<Input*>& aInputs );
    
    static Input* getNumeraireInput( const std::vector<Input*>& aInputs );
    
    static double getRho( const double aSigma );
    
    static double getNetPresentValueMult( const std::vector<Input*>& aInputs,
                                          const double aLifetimeYears );
    
    static double calcNetPresentValueMult( const double aDiscountRate,
                                           const double aLifetime );

    
    static double getExpectedPriceReceived( const std::vector<Input*>& aInputs,
                                            const std::string& aRegionName,
                                            const std::string& aGoodName,
                                            const double aLifetimeYears,
                                            const int aPeriod );
    
    static double applyTechnicalChangeInternal( std::vector<Input*>& input,
                                                const TechChange& aTechChange, 
                                                const std::string& regionName,
                                                const std::string& sectorName, 
                                                const int aPeriod,
                                                double alphaZero,
                                                double sigma );
};

#endif // _FUNCTION_UTILS_H_