#include "gateways.h"

int main(int argc, char** argv)
{
    //Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    GatewayWrapper gatewayWrapper(argv);

    return 0;
}