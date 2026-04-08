#include "config.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <getopt.h>  // for getopt_long

bool parse_arguments(int argc, char** argv, SimulationParams& params) {
    // Set defaults
    params = SimulationParams();
    
    // Define long options
    static struct option long_options[] = {
        {"nx", required_argument, 0, 'x'},
        {"ny", required_argument, 0, 'y'},
        {"steps", required_argument, 0, 's'},
        {"Lx", required_argument, 0, 'l'},
        {"Ly", required_argument, 0, 'm'},
        {"alpha", required_argument, 0, 'a'},
        {"boundary", required_argument, 0, 'b'},
        {"init", required_argument, 0, 'i'},
        {"Tleft", required_argument, 0, 'L'},
        {"Tright", required_argument, 0, 'R'},
        {"Tbottom", required_argument, 0, 'B'},
        {"Ttop", required_argument, 0, 'T'},
        {"Tinit", required_argument, 0, 'I'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int c;
    int option_index = 0;
    
    while ((c = getopt_long(argc, argv, "x:y:s:l:m:a:b:i:L:R:B:T:I:h", 
                            long_options, &option_index)) != -1) {
        switch (c) {
            case 'x':
                params.Nx = atoi(optarg);
                break;
            case 'y':
                params.Ny = atoi(optarg);
                break;
            case 's':
                params.num_steps = atoi(optarg);
                break;
            case 'l':
                params.Lx = atof(optarg);
                break;
            case 'm':
                params.Ly = atof(optarg);
                break;
            case 'a':
                params.alpha = atof(optarg);
                break;
            case 'b':
                params.boundary_type = atoi(optarg);
                break;
            case 'i':
                params.init_type = atoi(optarg);
                break;
            case 'L':
                params.T_left = atof(optarg);
                break;
            case 'R':
                params.T_right = atof(optarg);
                break;
            case 'B':
                params.T_bottom = atof(optarg);
                break;
            case 'T':
                params.T_top = atof(optarg);
                break;
            case 'I':
                params.T_initial = atof(optarg);
                break;
            case 'h':
                std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
                std::cout << "Options:" << std::endl;
                std::cout << "  --nx <value>      Grid size in x-direction" << std::endl;
                std::cout << "  --ny <value>      Grid size in y-direction" << std::endl;
                std::cout << "  --steps <value>   Number of time steps" << std::endl;
                std::cout << "  --Lx <value>      Physical domain length in x" << std::endl;
                std::cout << "  --Ly <value>      Physical domain length in y" << std::endl;
                std::cout << "  --alpha <value>   Thermal diffusivity" << std::endl;
                std::cout << "  --boundary <0|1>  Boundary type" << std::endl;
                std::cout << "  --init <0|1|2>    Initial condition" << std::endl;
                std::cout << "  --Tleft <value>   Left boundary temperature" << std::endl;
                std::cout << "  --Tright <value>  Right boundary temperature" << std::endl;
                std::cout << "  --Tbottom <value> Bottom boundary temperature" << std::endl;
                std::cout << "  --Ttop <value>    Top boundary temperature" << std::endl;
                std::cout << "  --Tinit <value>   Initial temperature" << std::endl;
                std::cout << "  --help            Show this help" << std::endl;
                return false;
            default:
                return false;
        }
    }
    
    return true;
}
