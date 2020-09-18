# Goober Eats

### Introduction and Program Usage

Goober Eats is a simplified navigation program which generates a delivery route for a food delivery robot which starts at a food delivery depot with a set of deliveries. The program attempts to minimize the total distance the robot must travel in order to save costs. In other words, it tries to produce an optimal route.

It takes as input two text files - one containing map data and the other containing a list of deliveries. The output is a guided tour with instructions on which street to drive on and for how much distance, turns to make and deliveries to complete.

The map data is represented by a number of streets with each street further broken up into street segments.

The format of the map data text file is as follows:

```
NAME OF STREET
NUMBER OF STREET SEGMENTS FOR THIS STREET (Say there are N segments)
N LINES EACH WITH TWO COORDINATES FOR EACH SEGMENT
```

The above is for one street. Multiple streets can be included in the file and the program will handle them appropriately.

The coordinates of the street segments are double values representing the latitude and longitude of the starting and ending point of each segment. For example, for 18th Street in Westwood, Los Angeles, the format of the data would be:

```
18th Street
6
34.0426164 -118.5001481 34.0411322 -118.4984104
34.0411322 -118.4984104 34.0395903 -118.4966053
34.0395903 -118.4966053 34.0379395 -118.4946729
34.0379395 -118.4946729 34.0363939 -118.4928633
34.0363939 -118.4928633 34.0353076 -118.4915916
34.0353076 -118.4915916 34.0352087 -118.4914758
```

The above data tells the program that 18th Street is composed of 6 street segments, which begin and end on the coordinates on the 6 lines below.

In this fashion, the data of a large number of streets and street segments can be put together to create an entire "map" of a region. A sample data file for Westwood is `mapdata.txt` in this repository.

The text file containing deliveries is structured a bit differently. On the first line are the coordinates of the food delivery depot from which the food delivery robot will begin its journey.

This is followed by `D` lines, each describing a delivery location and the item to be delivered, in the following format:

```
LATITUDE LONGITUDE:ITEM
```

A sample delivery file (`deliveries.txt` in this repository), describing deliveries in and around UCLA would be:

```
34.0625329 -118.4470263
34.0712323 -118.4505969:Chicken tenders (Sproul Landing)
34.0687443 -118.4449195:B-Plate salmon (Eng IV)
34.0685657 -118.4489289:Pabst Blue Ribbon beer (Beta Theta Pi)
```

Once the map data and delivery information text files are ready, building and running the program is fairly simple:

```
$ git clone https://github.com/ksj1602/goober-eats.git
$ make
$ ./goober [MAP DATA FILE] [DELIVERY DATA FILE]
```

### Technical Implementation Details

I have implemented my own expandable hash map, which can be initialized with a load factor. The default load factor is 0.5.

The implementation focuses on simplicity. If the ratio of the number of key-value pairs in the map to the number of "buckets" exceeds the load factor, then a new map is allocated with double the number of buckets and each key is rehashed and stored in the new hash map. Of course, this is a space-time tradeoff where time is being traded to conserve space, by delaying reallocation until it is absolutely needed.

This map is used to load the data for each street from the text file containing the map data. Each street name is a key with the values being the street segments associated with it. There is also a reverse mapping stored with street segments mapped to street names, which is used for route construction.

For the deliveries, point to point routing is achieved with the use of Dijkstra's Algorithm to get the shortest distance.

Finally, once the route is established, it is converted to directions in English before being printed out to standard output.