#include "Application.hpp"

int main(int argc, char** argv)
{
	srand (time(NULL));
    Application app(argc, argv);
    return app.run();
}
