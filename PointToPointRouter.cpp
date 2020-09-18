#include "provided.h"
#include "ExpandableHashMap.h"
#include <list>
#include <limits>
#include <iostream>
#include <set>
using namespace std;

#define MAX_DOUBLE numeric_limits<double>::max()

// declaring additional struct for implementing Dijkstra's Algorithm
// This additional functionality is needed to elegantly implement generatePointToPointRoute
struct processCoord {

    // the location of a processCoord is that of the GeoCoord it represents
    GeoCoord location;

    // variable required for Dijkstra's algorithm implementation
    double distanceFromSource = MAX_DOUBLE; // initializing all distances to infinity by default

    // < operator defined to use sets of processCoords, ordered by distance
    bool operator<(const processCoord &rhs) const {
        return (this->distanceFromSource < rhs.distanceFromSource);
    }
};

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;

private:
    const StreetMap* m_streetMap;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
    : m_streetMap(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl() = default;

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    /*
     * This function uses an adapted form of Dijkstra's Algorithm to find the shortest path from one GeoCoord to another
     */

    // if the start and end coordinates are equal, we simply return after setting the arguments to correct values
    if (start == end) {
        route.clear();
        totalDistanceTravelled = 0;
        return DELIVERY_SUCCESS;
    }

    // if one or both of start and end do not exist in our map data, return BAD_COORD
    vector<StreetSegment> checker; // a vector used to call getStreetSegmentsThatStartWith to determine if a bad coordinate has been passed
    if (!m_streetMap->getSegmentsThatStartWith(start, checker) || !m_streetMap->getSegmentsThatStartWith(end, checker))
        return BAD_COORD;

    // initializing the source vertex
    processCoord source;
    source.location = start;
    source.distanceFromSource = 0;

    set<processCoord> processed; // set to store those coordinates which have already been processed
    vector<StreetSegment> processor; // vector to get set of neighbors of the current coordinate (in call to getStreetSegmentsThatStartWith)
    set<processCoord> vertexQueue; // set to represent priority queue on which to run Dijkstra's Algorithm
    
     // hashmap to store distance from the source to the coordinates for which we have some initial value
    ExpandableHashMap<GeoCoord, double> distanceMap;
    
    // hashmap to store how the destination has to be reached, by associating each location with the segment is must be reached from
    ExpandableHashMap<GeoCoord, StreetSegment> routeMap;

    // initially, we insert only the source vertex into the priority queue
    vertexQueue.insert(source);
    
    // the distance to the source is obviously, zero
    distanceMap.associate(source.location, 0);

    // Dijkstra Processing
    while (!vertexQueue.empty()) {

        // get the closest vertex to source. A set will store the vertices in sorted order
        processCoord current = *(vertexQueue.begin());

        // remove it from the priority queue
        vertexQueue.erase(vertexQueue.begin());

        /*
         * if we have already processed a vertex, do not process it again
         * Notice how there is a difference between visiting a vertex and processing it
         * A vertex will be visited each time a neighbor of itself is processed
         * Whereas it will itself be processed only once
        */
        
        
        // if the current vertex exists in our 'processed' set, find() will return an iterator other than end()
        if (processed.find(current) != processed.end())
            continue;


        //If this is a new vertex to be processed, insert it into the processed set
        processed.insert(current);

        // If the current vertex is the destination vertex, we update the arguments to appropriate values and return
        if (current.location == end) {

            // the total distance travelled is the distance from the source vertex
            totalDistanceTravelled = *(distanceMap.find(end));

            // clear out the route parameter first so that there are no unnecessary/incorrect segments already in it
            route.clear();

            // backtrack using the routeMap
            GeoCoord routeConstructor = current.location;
            while (routeConstructor != start) {
                
                StreetSegment toBeAdded = *(routeMap.find(routeConstructor));

                // using push_front so that the route is in order, since in our loop we're going from destination -> source
                route.push_front(toBeAdded);
                
                // update routeConstructor to the previous GeoCoord on our route
                routeConstructor = toBeAdded.start;
            }
            return DELIVERY_SUCCESS;
        }

        // if we are not at the destination point yet, get all neighbors of the current vertex
        // if the location is not in our map, return BAD_COORD
        if (!m_streetMap->getSegmentsThatStartWith(current.location, processor))
            return BAD_COORD;

        // update distances from source of all neighbors if required
        for (const auto &x : processor) { // for all the street segments beginning at a particular vertex

            // convert the end of the StreetSegment (in other words, neighbor of the current vertex) from a GeoCoord to a processCoord
            processCoord temp;
            temp.location = x.end;
            
            // distance used to compare needs to be stored in a variable
            // we may already have computed an initial distance to this location, in which case we take that value for comparison
            double comparisonDistance;
            if (distanceMap.find(temp.location) != nullptr) {
                comparisonDistance = *(distanceMap.find(temp.location));
            }
            
            // if we have not already computed a value, we take the default value (infinity)
            else
                comparisonDistance = temp.distanceFromSource;

            // compute the value of a new possible distance
            double possibleNewDistance = distanceEarthMiles(current.location, x.end) + current.distanceFromSource;

            // if the new distance is shorter than the original one, update appropriate fields
            if (possibleNewDistance < comparisonDistance) {
                temp.distanceFromSource = possibleNewDistance;
                distanceMap.associate(temp.location, possibleNewDistance);
                routeMap.associate(temp.location, x);
            }

            // insert the current neighbor into the queue so that it can be processed
            vertexQueue.insert(temp);
        }

        // clear out the vector of street segments so that the next vertex can be processed
        processor.clear();
    }

    // NO_ROUTE returned when after all the processing, we could not find a route from source to destination
    return NO_ROUTE;
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
