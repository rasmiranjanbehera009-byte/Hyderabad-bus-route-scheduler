# Hyderabad Bus Route Scheduler

A C-based console application that models a real bus network around Secunderabad, Hyderabad — letting users view live bus schedules, track waiting passengers, and calculate the fastest route between any two stops using Dijkstra's algorithm.

## Data Structures Used

- **Graph (Adjacency List)** — models the bus stop network; each stop holds a linked list of edges (destination, travel time, route number)
- **Circular Queue** — manages live bus arrivals at Secunderabad Bus Stand as a fixed-size ring buffer (front/rear pointers, wraps around when full)
- **Linked List (Passenger Queue)** — stores passengers waiting at the stand, each with a destination stop
- **Min-Heap + Dijkstra's Algorithm** — computes the shortest travel time (in minutes) between any two of the 15 modeled stops

## Features

- **View Network Graph** — prints the full adjacency list of all stops and their connected routes
- **View Arrival Schedule** — shows a live circular queue of buses (ID, route, arrival time, capacity, passenger count, on-time/delayed status)
- **View Passenger Queue** — displays all waiting passengers and their destinations as a linked list
- **Find Shortest Path** — runs Dijkstra's algorithm (with a min-heap) to compute the fastest route and travel time between any two stops, plus a full distance table from the source stop
- **Simulate Bus Boarding** — boards waiting passengers onto the next bus in queue based on available seats
- **Schedule a New Bus** — enqueues a new bus into the circular queue
- **Add a Passenger** — enqueues a new passenger with a chosen destination
- **Add a Route** — adds a new bidirectional edge (route) to the graph
- **Dequeue Next Bus** — processes the departure of the next bus in the queue

## Modeled Stops

Includes 15 real stops around Secunderabad — Secunderabad Bus Stand, Paradise Circle, Begumpet, Ameerpet, SR Nagar, Balkampet, Trimulgherry, Mettuguda, Tarnaka, Uppal, ECIL X Roads, Habsiguda, Musheerabad, Gandhi Hospital, and Koti — pre-loaded with realistic travel times and routes.

## Tech Stack

- **C** (standard library only — `stdio.h`, `stdlib.h`, `string.h`, `limits.h`)
- Custom implementations of a graph, circular queue, singly linked list, and binary min-heap (no external libraries)

## How to Compile & Run

```bash
gcc BUS_tracking_system.c -o bus_scheduler
./bus_scheduler
```

## Sample Use Case

Selecting **"Find Shortest Path"** from Musheerabad to Secunderabad Stand returns the fastest path (via Paradise Circle) along with the total travel time, plus a full table of travel times to every other stop from the source — all computed live using Dijkstra's algorithm.

## What I Learned

Building this project gave me hands-on practice implementing core data structures from scratch in C — including a circular queue with wraparound logic, a singly linked list, a binary min-heap, and Dijkstra's shortest-path algorithm — instead of relying on built-in library structures.
