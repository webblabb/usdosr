USDOSv2.1
===
A simplified overview of the processes executed in "main.cpp".

1. Load configuration file @File_manager
2. Set parameters
3. Load premises @Grid_manager
4. Create grid @Grid_manager
5. Load/generate list of initial infections
6. Loop through number of replicates
  7. Initialize control actions @Control_actions
  8. Initialize Status Manager, set initial infections @Status_manager
  9. Initialize Shipment Manager @Shipment_manager
  10. Initialize Grid Checker @Grid_checker
  11. Loop through timesteps or until epidemic dies
    12. Advance timestep
    13. Update disease statuses @Status_manager
    14. Update control statuses @Control_actions
    15. Evaluate local spread @Grid_checker
    16. Generate shipments @Shipment_manager
    17. Evaluate shipment spread @Status_manager, @Control_actions
    18. (Print transmission details for this timestep)
  19. (Print summary for this replicate)
End main simulation loop

## Generating documentation
- Download and install Doxygen at: http://www.stack.nl/~dimitri/doxygen/download.html
- Run the doxywizard
- From the File menu, choose Open, and find the doxyfile within this repository. The configuration settings will be loaded into the doxywizard
- Run to generate documentation.
