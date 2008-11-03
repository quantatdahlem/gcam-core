#ifndef _WORLD_H_
#define _WORLD_H_
#if defined(_MSC_VER)
#pragma once
#endif

/*
 * LEGAL NOTICE
 * This computer software was prepared by Battelle Memorial Institute,
 * hereinafter the Contractor, under Contract No. DE-AC05-76RL0 1830
 * with the Department of Energy (DOE). NEITHER THE GOVERNMENT NOR THE
 * CONTRACTOR MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY
 * LIABILITY FOR THE USE OF THIS SOFTWARE. This notice including this
 * sentence must appear on any copies of this computer software.
 * 
 * EXPORT CONTROL
 * User agrees that the Software will not be shipped, transferred or
 * exported into any country or used in any manner prohibited by the
 * United States Export Administration Act or any other applicable
 * export laws, restrictions or regulations (collectively the "Export Laws").
 * Export of the Software may require some form of license or other
 * authority from the U.S. Government, and failure to obtain such
 * export control license may result in criminal liability under
 * U.S. laws. In addition, if the Software is identified as export controlled
 * items under the Export Laws, User represents and warrants that User
 * is not a citizen, or otherwise located within, an embargoed nation
 * (including without limitation Iran, Syria, Sudan, Cuba, and North Korea)
 *     and that User is not otherwise prohibited
 * under the Export Laws from receiving the Software.
 * 
 * All rights to use the Software are granted on condition that such
 * rights are forfeited if User fails to comply with the terms of
 * this Agreement.
 * 
 * User agrees to identify, defend and hold harmless BATTELLE,
 * its officers, agents and employees from all liability involving
 * the violation of such Export Laws, either directly or indirectly,
 * by User.
 */


/*! 
* \file world.h
* \ingroup Objects
* \brief The World class header file.
* \author Sonny Kim
*/

#include <map>
#include <vector>
#include <list>
#include <memory>
#include <xercesc/dom/DOMNode.hpp>
#include "util/base/include/ivisitable.h"
#include "util/base/include/iround_trippable.h"

// Forward declarations
class Region;
class ILogger;
class Curve;
class CalcCounter;
class IClimateModel;
class GHGPolicy;
class GlobalTechnologyDatabase;

namespace objects {
    class Atom;
}
template <class T, class U> class HashMap;

/*! 
* \ingroup Objects
* \brief A class which contains all the model's regions.  These regions may be MiniCAM (partial
* equilibrium) regions or SGM (general equilibrium) regions as they are derived
* from the Region base class.
*
* The World class object is contained by the Scenario class object.  The world object controls
* the calling of the regions which it has been told to solve (passed in an
* argument of the method world.calc()) by calling region.calc() to run the model
* for one iteration for these regions.
*
* The world object includes a switch for running the model in calibration mode,
* methods for determining the chain of sector dependecies (necessary for solving
* partial equilibrium model), and the methods for setting global fixed GHG
* taxes.
* \author Sonny Kim
*/

class World: public IVisitable, public IRoundTrippable
{
public:
    World();
    ~World();
    void XMLParse( const xercesc::DOMNode* node );
    void completeInit();
    void toInputXML( std::ostream& out, Tabs* tabs ) const;
    void toDebugXML( const int period, std::ostream& out, Tabs* tabs ) const;
	static const std::string& getXMLNameStatic();
    const std::string& getName() const;
    void initCalc( const int period );
    void postCalc( const int aPeriod );

    //! The type of the vector containing region atoms.
    typedef std::vector<const objects::Atom*> AtomVector;
    void calc( const int period, const AtomVector& aRegionsToCalc = AtomVector() );
    void updateSummary( const std::list<std::string> aPrimaryFuelList, const int period ); 
    void runClimateModel();
    void csvOutputFile() const; 
    void dbOutput( const std::list<std::string>& aPrimaryFuelList ) const; 
    const std::map<std::string,int> getOutputRegionMap() const;
    const AtomVector getRegionIDs() const;
    void turnCalibrationsOn(); 
    void turnCalibrationsOff();
    bool getCalibrationSetting() const;
    bool isAllCalibrated( const int period, double calAccuracy, const bool printWarnings ) const;
    void setTax( const GHGPolicy* aTax );
    const IClimateModel* getClimateModel() const;
    const std::map<const std::string, const Curve*> getEmissionsQuantityCurves( const std::string& ghgName ) const;
    const std::map<const std::string, const Curve*> getEmissionsPriceCurves( const std::string& ghgName ) const;
    void setCalcCounter( CalcCounter* calcCounter );

	void accept( IVisitor* aVisitor, const int aPeriod ) const;
    void csvSGMOutputFile( std::ostream& aFile, const int period ) const;
    void csvSGMGenFile( std::ostream& aFile ) const;
private:
    //! The type of an iterator over the Region vector.
    typedef std::vector<Region*>::iterator RegionIterator;

    //! The type of a constant iterator over the Region vector.
    typedef std::vector<Region*>::const_iterator CRegionIterator;

    //! The type of the fast Region lookup hashmap.
    typedef HashMap<const objects::Atom*, unsigned int> FastRegionLookupMap;
    
    //! A fast hashmap which stores a mapping of region ID atom to region
    //! location. This allows world calc calls for derivatives to be faster.
	std::auto_ptr<FastRegionLookupMap> mRegionLookupMap;
    
    std::map<std::string, int> regionNamesToNumbers; //!< Map of region name to indice used for XML parsing.
    std::vector<Region*> regions; //!< array of pointers to Region objects
    std::auto_ptr<IClimateModel> mClimateModel; //!< The climate model.

    bool doCalibrations; //!< turn on or off calibration routines
    std::auto_ptr<GlobalTechnologyDatabase> globalTechDB; //!< The Global Technology Database.

    //! Reference to an object which maintains a count of the number of time
    //! calc() has been called.
    CalcCounter* calcCounter;

    void initAgLu(); 
    void clear();

    const std::vector<unsigned int> getRegionIndexesToCalculate( const AtomVector& aRegionsToCalc );
    void createFastLookupMap();
    void csvGlobalDataFile() const; 
    bool checkCalConsistancy( const int period );
};

#endif // _WORLD_H_