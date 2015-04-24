USDOSv2.1
===
Make sure the premises file doesn't have numbers in 1e+05 format (may happen with IDs)
A simplified overview of the functions/classes that are called, in order.

## Pre-simulation...
#### Grid_manager
- Reads in files containing premises and creates a grid over those premises by:
  1. using specified input parameters to create grid cells according to density,
  2. using specified input parameters to create grid cells of uniform sizes, or
  3. reading in a file containing pre-defined cells

## Start main simulation loop...
#### Control_actions
- Establish control action rules.

####Status_manager
- Reads in list of seed farms that begin the simulation infectious. Copies farms into Status_manager and sets each status to "exposed," with fixed start and end times.

### For each time step...
####Status_manager
- Update any farm due to change status at this time. Assign start and end times for next status.
- Print updates to console.
- Make lists of farms that are infectious and no-longer susceptible, to be fed into Grid_checker.

####Grid_checker
- Evaluates localized infection spread via the gridding method. Returns pointers to newly exposed original farms.

####Shipment_manager
- Creates shipments, first at the county level (this part is USAMM), then assigns those to individual farms by:
  1. randomly distributing shipments among farms in both origin and destination counties
  2. assigning shipments with probabilities based on relative (to county) sizes of farms
  3. assigning the largest farm in the origin county to ship to random destinations in destination county
  4. assigning random farms in the origin county to ship to the largest farm in the destination county
  5. assigning all shipments to go from the largest farm in the origin county to the largest farm in the destination county
- Returns pointers to newly exposed original farms

####Status_manager
- Check with Control_actions if any localized spread from Grid_checker should be blocked
- Determines which shipments from Shipment_manager originated from infectious farms
- Check with Control_actions if any shipments should be blocked
- Copies any farms not already copied into Status_manager, adds new exposures
- If detailed output is on, get source lists from Grid_checker and Shipment_manager and write out.

### End time step...

####Status_manager
- If summary output is on, write out summary output for this replicate

## End main simulation loop
