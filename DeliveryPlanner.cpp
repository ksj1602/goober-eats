#include "provided.h"
#include <vector>
#include <list>
using namespace std;

string stringAngleForProceed (double angle);

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
    
    // this is a helper function for generateDeliveryPlan, see function implementation for details
    DeliveryResult navigate (const GeoCoord& start, const GeoCoord& end, vector<DeliveryCommand>& commands, double& totalDistanceTravelled) const;
private:
    
    // this StreetMap pointer isn't very helpful for this class but it's there because the spec required it
    const StreetMap* m_streetMap;
    
    // a PointToPointRouter is required to generate routes to/from the depot and/or delivery locations
    const PointToPointRouter* m_ptopRouter;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
    : m_streetMap(sm)
{
    m_ptopRouter = new PointToPointRouter(sm);
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
    // free the memory allocated for the PointToPoint router
    delete m_ptopRouter;
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    // currently generates delivery commands without optimization

    // local variables required to use generatePointToPointRoute
    GeoCoord currentStart = depot;
    GeoCoord currentDestination;
    DeliveryCommand currentCommand;
    totalDistanceTravelled = 0;
    DeliveryResult r;


    for (auto &x : deliveries) {
        
        // the current destination for our algorithm is always the next entry in the deliveries vector
        currentDestination = x.location;
        
        // we use this small check here to prevent memory allocation for variables and processing long paths
        // so that the process is sped up
        if (currentStart == currentDestination) {
            currentCommand.initAsDeliverCommand(x.item);
            commands.push_back(currentCommand);
            currentStart = currentDestination;
            continue;
        }
        
        // navigate will add the required commands to the vector and update totalDistanceTravelled accordingly
        // if there is a bad result returned, we convey that to the caller
        r = navigate(currentStart, currentDestination, commands, totalDistanceTravelled);
        if (r == NO_ROUTE || r == BAD_COORD)
            return r;

        // at this point we have reached the delivery location, so we append the appropriate command
        currentCommand.initAsDeliverCommand(x.item);
        commands.push_back(currentCommand);
        
        // update currentStart for the next delivery
        currentStart = currentDestination;
    }

    // now that we have completed all the deliveries, we must provide commands from the last delivery location back to the depot
    // currentStart already holds the location of the last delivery location
    r = navigate(currentStart, depot, commands, totalDistanceTravelled);
    
    // whatever is returned by the previous statement is the result of our function which we return to the caller
    return r;
}

/*
 This function "navigates" from the start coordinate to the end coordinate
 In the process, it generates appropriate commands and adds them to the vector of commands
 In other words, some of the work to be done by generateDelivery Plan has been factored out here
 This particular function takes care of getting a point to point route and then converting the route into commands
 The commands are then added to the vector
 */
DeliveryResult DeliveryPlannerImpl::navigate(const GeoCoord &start,
        const GeoCoord &end,
        vector<DeliveryCommand> &commands,
        double &totalDistanceTravelled) const {
    
    // there are no commands to be generated, so we simply return DELIVERY_SUCCESS
    if (start == end)
        return DELIVERY_SUCCESS;
    
    // local variables required for processing and calling PointToPointRoute functions
    double distance;
    list<StreetSegment> currentRoute;
    
    // local variable required to hold the current command being computed
    DeliveryCommand currentCommand;
    
    // we try to get a route from the start coordinate to the end coordinate and store it in a temporary result variable
    DeliveryResult tempResult = m_ptopRouter->generatePointToPointRoute(start, end, currentRoute, distance);
    
    // in case a bad result is returned, we convey that to the caller
    if (tempResult == NO_ROUTE || tempResult == BAD_COORD)
        return tempResult;
    
    // these iterators are required to iterate through the route of street segments
    // the tempIt is used when on a route, we change streets and so need to know the angle of the change to determine a turn
    list<StreetSegment>::iterator it, tempIt;
    
    // at this point, no bad value has been returned by generatePointToPointRoute
    // so we can safely add the distance it returned to totalDistanceTravelled
    totalDistanceTravelled += distance;
    
    // we now iterate through every street segment of the route
    // the iterator will be updated in the body of the loop
    for (it = currentRoute.begin(); it != currentRoute.end(); ) {
        
        // we store the name of the current street we are on so that proceed commands are not duplicated
        string temp = it->name;
        
        // give an initial value to the current proceed command
        // the stringAngleForProceed command does the job giving the direction corresponding to the angle of the segment
        currentCommand.initAsProceedCommand(stringAngleForProceed(angleOfLine(*it)), temp, distanceEarthMiles(it->start, it->end));
        
        // the job of the tempIt iterator is to point to the last segment of the current street
        // right now that value is pointed to by 'it'
        tempIt = it;
        
        // as long as we are on the same street, keep adding distance to the proceed command
        while (++it != currentRoute.end() && it->name == temp) {
            currentCommand.increaseDistance(distanceEarthMiles(it->start, it->end));
            
            // update tempIt to reflect the last segment of the current street
            tempIt = it;
        }
        
        // the proceed command was fully ready so we can now add it to the vector
        commands.push_back(currentCommand);
        
        // if we have changed streets, we must compute a turn command and add it to the vector
        if (it != currentRoute.end()) {
            
            // compute an angle between the previous street and the new one
            // this is where the tempIt iterator comes handy
            // 'it' now points to the next street while 'tempIt' still holds the last segment of the previous street
            double switchAngle = angleBetween2Lines(*tempIt, *it);
            
            // computing and adding the turn command as given in the spec
            if (switchAngle >= 1 && switchAngle < 180) {
                currentCommand.initAsTurnCommand("left", it->name);
                commands.push_back(currentCommand);
            }
            else if (switchAngle >= 180 && switchAngle <= 359) {
                currentCommand.initAsTurnCommand("right", it->name);
                commands.push_back(currentCommand);
            }
        }
    }
    
    // a route was found and successfully converted to commands
    return DELIVERY_SUCCESS;
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}



// Auxiliary function implementation
string stringAngleForProceed (double angle) {
    
    // dividing the angle by 22.5 gives us the direction
    int p = (int) (angle / 22.5);
    switch (p) {
        case 0:
        case 15:
            return "east";
        case 1:
        case 2:
            return "northeast";
        case 3:
        case 4:
            return "north";
        case 5:
        case 6:
            return "northwest";
        case 7:
        case 8:
            return "west";
        case 9:
        case 10:
            return "southwest";
        case 11:
        case 12:
            return "south";
        case 13:
        case 14:
            return "southeast";
        default:
            break;
    }
    return "";
}
