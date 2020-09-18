#include "provided.h"
#include "ExpandableHashMap.h"
#include <string>
#include <vector>
#include <fstream>
#include <functional>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
    
private:
    
    // our map of GeoCoords to the street segments that start with them
    ExpandableHashMap<GeoCoord, vector<StreetSegment> > m_coordToSegmentMap;
    
    // helper function to insert a street segment into the map
    void insertSegment(StreetSegment* s) {
        
        // Declaring a vector pointer to call find
        vector<StreetSegment> *f = m_coordToSegmentMap.find(s->start);
        
         // if coordinate already exists, we add this segment to its vector of segments
        if (f != nullptr)
            f->push_back(*s);
        
        // otherwise we create a new vector and add the coordinate and this vector into the hashmap
        else {
            vector<StreetSegment> temp;
            temp.push_back(*s);
            m_coordToSegmentMap.associate(s->start, temp);
        }
        
        delete s; // delete the dynamically allocated street segment to prevent a memory leak
    }
};

StreetMapImpl::StreetMapImpl()
{
    // empty constructor, there is nothing to initialize
}

StreetMapImpl::~StreetMapImpl()
{
    // empty destructor, there is nothing to deallocate
}

bool StreetMapImpl::load(string mapFile)
{
    // initialize the input file stream
    ifstream mapDataFile(mapFile);
    
    // if there was an error, convey that to the caller by returning false
    if (!mapDataFile)
        return false;
    
    // variables required to collect input
    string streetName;
    int numberOfSegments;
    string start1, start2;
    string end1, end2;
    
    // while there are street segments in the input file
    while (getline(mapDataFile, streetName)) {
        
        // read number of street segments for the current street being processed
        mapDataFile >> numberOfSegments;
        
         // after reading an integer, discard the rest of the input on the line
        mapDataFile.ignore(1000, '\n');
        
        // for each segment, read in its start and end coordinates
        // and then add those to our hashmap
        for (int i = 0; i < numberOfSegments; i++) {
            
            // read in coordinates
            mapDataFile >> start1 >> start2 >> end1 >> end2;
            
            // discard remaining line input
            mapDataFile.ignore(1000, '\n');
            
            // create two temporary GeoCoord variables needed to construct the street segments
            GeoCoord new1(start1, start2);
            GeoCoord new2(end1, end2);
            
            // call insertSegment() to insert the segments into our map
            // these dynamically allocated segments are deallocated by insertSegment
            insertSegment(new StreetSegment(new1, new2, streetName));
            insertSegment(new StreetSegment(new2, new1, streetName));
        }
    }
    
    // Street map successfully loaded
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    // call the find function of the ExpandableHashMap to get a pointer to the vector of street segments
    const vector<StreetSegment> *f = m_coordToSegmentMap.find(gc);
    
    // if there are no segments which begin with the given coordinate, convey that to the caller
    if(f == nullptr)
        return false;
    
    // update the value of the argument appropriately
    // if the segs vector already had some contents, they are automatically erased by the assignment operator
    segs = *f;
    
    return true;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
