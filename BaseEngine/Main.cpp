#include "Application.h"
#include "Demo.h"


// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 1080;

int main()
{
    Demo app;

    app.Init(SCR_WIDTH, SCR_HEIGHT, "Base Engine");
    app.Run();

    return 0;
}