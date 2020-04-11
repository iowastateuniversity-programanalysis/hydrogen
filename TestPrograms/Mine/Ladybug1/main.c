#include <stdio.h>

// A ladybug has 3 dots per wing
int get_num_dots(int num_wings) {
    int num_dots = num_wings * 3;
    return num_dots;
}

// A ladybug has 3 dots per wing
int main() {
    int num_wings, num_dots;
    scanf("%d", &num_wings);

    if (num_wings <= 0) {
        printf("A ladybug gotta have wings, ya dig?\n");
    }

    num_dots = num_wings * 3;

    printf("A %d-winged ladybug has %d dots\n", num_wings, num_dots);
}
