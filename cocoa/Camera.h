#ifndef _SPB_CAMERA
#define _SPB_CAMERA

#include <Node.h>

namespace SPB
{
    class Camera : public Node {
        public:
            Camera();
            ~Camera();

            void setPerspective();
    };
};

#endif
