#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cbmp.h"
#include <time.h>
#include "variables.h"
#include "pattern.h"

//Wall = warning all
//To compile: gcc cbmp.c pattern.c Algorithm.c -o .\Algorithm.exe -std=c99 -Wall -O2
/*To run: 

.\Algorithm.exe .\samples\easy\1EASY.bmp .\samples\easy\1EASYOutErosionTest.bmp e

.\Algorithm.exe .\samples\hard\4HARD.bmp .\samples\hard\4HARDOutErosionTest.bmp e

    If you want pattern search: 

.\Algorithm.exe .\samples\easy\1EASY.bmp .\samples\easy\1EASYOutPattern.bmp p

*/

/*int structuring_element[3][3] = {
    {0, 1, 0},
    {1, 1, 1},
    {0, 1, 0}};*/
char neighbours[neighbourColumn][neighbourRow] = {
    {0, -1},
    {0, 1},
    {-1, 0},
    {1, 0}};

void increment_cell_count()
{
    countedCells++;
}

void grey_scale_image(unsigned char image[BMP_WIDTH][BMP_HEIGTH], unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS])
{
    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            image[x][y] = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
        }
    }
}

void binary_threshold(unsigned char image[BMP_WIDTH][BMP_HEIGTH])
{
    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            if (image[x][y] <= treshold_value)
            {
                image[x][y] = 0;
            }
            else
            {
                image[x][y] = 1;
            }
        }
    }
}

char inside_bounds(int x, int y)
{
    if (x > BMP_WIDTH - 1 || y > BMP_HEIGTH - 1 || x < 0 || y < 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void add_coordinates__to_array(int x, int y)
{
    coordinatesArray[nextCor].x = x;
    coordinatesArray[nextCor].y = y;
    nextCor++;
}

void print_coordinates()
{
    for (int i = 0; i < 950; i++)
    {
        if (coordinatesArray[i].x == 0 && coordinatesArray[i].y == 0)
        {
            //printf("%d",i);
            //return;
        }
        else
        {
            printf("{%d,%d}\n", coordinatesArray[i].x, coordinatesArray[i].y);
        }
    }
}

void mark_cell(int x, int y)
{
    add_coordinates__to_array(x, y);

    for (char cross = -8; cross <= 8; cross++)
    {
        for (char thickness = -1; thickness <= 1; thickness++)
        {
            if (inside_bounds(x + cross, y + thickness) == 1)
            {
                input_image[x + cross][y + thickness][0] = 255;
                input_image[x + cross][y + thickness][1] = 0;
                input_image[x + cross][y + thickness][2] = 0;
            }

            if (inside_bounds(x + thickness, y + cross) == 1)
            {
                input_image[x + thickness][y + cross][0] = 255;
                input_image[x + thickness][y + cross][1] = 0;
                input_image[x + thickness][y + cross][2] = 0;
            }
        }
    }
}

//remove the found cell and replaces it with black pixels
void remove_cell(int x, int y, char given_layer, unsigned char image[BMP_WIDTH][BMP_HEIGTH])
{
    for (char width = -given_layer; width <= given_layer; width++)
    {
        for (char height = -given_layer; height <= given_layer; height++)
        {
            if (inside_bounds(x + width, y + height))
            {
                image[x + width][y + height] = 0;
            }
        }
    }
}

char check_width(unsigned char image[BMP_WIDTH][BMP_HEIGTH], int x, int y, char currentLayer)
{
    for (char width = 0; width < currentLayer; width++)
    {

        if (inside_bounds(x + width, y - currentLayer) && image[x + width][y - currentLayer] == 1)
        {
            return 1;
        }

        if (inside_bounds(x + width, y + currentLayer) && image[x + width][y + currentLayer] == 1)
        {
            return 1;
        }
    }

    return 0;
}

char check_height(unsigned char image[BMP_WIDTH][BMP_HEIGTH], int x, int y, char currentLayer)
{

    for (char height = 0; height < currentLayer; height++)
    {

        if (inside_bounds(x - currentLayer, y + height) && image[x - currentLayer][y + height] == 1)
        {
            return 1;
        }

        if (inside_bounds(x + currentLayer, y + height) && image[x + currentLayer][y + height] == 1)
        {
            return 1;
        }
    }

    return 0;
}

char not_too_close(int x, int y)
{
    if (nextCor == 0)
    {
        return 1;
    }

    for (int i = 0; i < nextCor; i++)
    {
        int distance = (x - coordinatesArray[i].x) * (x - coordinatesArray[i].x) + (y - coordinatesArray[i].y) * (y - coordinatesArray[i].y);
        if (sqrt(distance) < allowedProximity)
        {
            return 0;
        }
    }

    return 1;
}

void find_white_neighbours(unsigned char image[BMP_WIDTH][BMP_HEIGTH], int x, int y, char currentLayer)
{
    if (currentLayer <= 1)
    {
        return;
    }

    char found = 0;
    found = check_width(image, x, y, currentLayer); //width

    if (found == 0)
    {
        found = check_height(image, x, y, currentLayer); //height
    }

    if (found == 1)
    {
        find_white_neighbours(image, x, y, currentLayer - 1);
    }
    else
    {
        remove_cell(x, y, currentLayer, image);

        if (not_too_close(x, y) == 1) //only marks and counts the cell, if it is more than a few pixels from the last recorded cell
        {
            mark_cell(x, y);
            countedCells++;
        }
    }
}

void capture_cells(unsigned char image[BMP_WIDTH][BMP_HEIGTH])
{
    //capture and mark potential cells

    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            if (image[x][y] == 1)
            {
                find_white_neighbours(image, x, y, REMOVE_CELL_SIZE); //starts marking and removing captured cells
            }
        }
    }
}

//Iterates over the image. If it finds a white pixel, it checks the four neighbours directly above, below, left and right
//and if any are black, it itself becomes black. It then increments if there are no other sorrounding white pixels in 12 by 12 at all.
void erosion(unsigned char image[BMP_WIDTH][BMP_HEIGTH])
{
    unsigned char nextImage[BMP_WIDTH][BMP_HEIGTH];
    char erosionsTics = 1;
    while (erosionsTics > 0)
    {
        totalErosions++;
        erosionsTics = 0;
        for (int x = 0; x < BMP_WIDTH; x++)
        {
            for (int y = 0; y < BMP_HEIGTH; y++)
            {
                if (image[x][y] == 1)
                {
                    char count = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        if (inside_bounds(x + neighbours[i][0], y + neighbours[i][1]) == 1 && image[x + neighbours[i][0]][y + neighbours[i][1]] == 1)
                        {
                            count++;
                        }
                    }

                    if (count != 4)
                    {
                        //edits image for next iterations
                        nextImage[x][y] = 0;
                        erosionsTics = 1;
                    }
                    else
                    {
                        nextImage[x][y] = 1;
                    }
                }
                else
                {
                    nextImage[x][y] = 0;
                }
            }
        }

        //This should be made faster.
        for (int x = 0; x < BMP_WIDTH; x++)
        {
            for (int y = 0; y < BMP_HEIGTH; y++)
            {
                image[x][y] = nextImage[x][y];
            }
        }

        
        
        capture_cells(image);
        
    }
}

/*
    Patterns
*/
/*

#define templateLength1 92.211713
#define templateLength2 287.459562
#define templateLength3 208.808525
#define CELL_PATTERN_SIZE 23

int grey_cell_pattern[CELL_PATTERN_SIZE][CELL_PATTERN_SIZE] = {
    {62, 55, 53, 55, 58, 61, 69, 90, 108, 131, 140, 142, 134, 116, 96, 73, 61, 61, 57, 58, 61, 59, 57},
    {60, 55, 58, 63, 76, 95, 115, 134, 163, 186, 198, 201, 192, 176, 142, 111, 88, 68, 62, 61, 59, 60, 60},
    {62, 58, 64, 86, 117, 167, 196, 212, 219, 225, 230, 235, 233, 226, 212, 187, 142, 90, 66, 64, 60, 60, 60},
    {65, 65, 85, 116, 164, 209, 236, 242, 245, 246, 247, 251, 252, 250, 242, 226, 193, 139, 100, 77, 65, 61, 57},
    {64, 76, 111, 166, 203, 228, 240, 247, 247, 246, 244, 249, 251, 252, 249, 243, 233, 215, 175, 106, 70, 60, 58},
    {74, 100, 144, 193, 221, 233, 240, 244, 245, 244, 245, 249, 251, 251, 251, 251, 249, 248, 217, 142, 94, 70, 59},
    {81, 123, 167, 200, 218, 227, 235, 242, 240, 238, 241, 247, 249, 249, 247, 247, 249, 251, 233, 195, 138, 84, 60},
    {94, 149, 180, 202, 214, 221, 229, 233, 234, 233, 237, 244, 247, 243, 241, 245, 246, 251, 242, 224, 178, 105, 69},
    {100, 154, 182, 196, 208, 219, 223, 221, 227, 231, 237, 241, 242, 236, 237, 239, 243, 246, 244, 238, 201, 128, 86},
    {99, 153, 183, 194, 203, 215, 218, 215, 218, 224, 229, 238, 239, 239, 237, 237, 240, 246, 248, 245, 214, 150, 97},
    {96, 148, 178, 190, 200, 208, 215, 213, 212, 218, 223, 232, 237, 241, 240, 237, 240, 249, 252, 250, 219, 151, 96},
    {87, 134, 162, 181, 198, 205, 212, 209, 211, 214, 219, 227, 234, 240, 240, 237, 242, 248, 252, 249, 213, 135, 89},
    {78, 107, 144, 171, 188, 197, 205, 210, 214, 214, 221, 229, 233, 233, 239, 242, 246, 248, 248, 242, 207, 122, 80},
    {71, 92, 126, 162, 182, 189, 198, 208, 215, 213, 218, 225, 229, 233, 235, 241, 246, 248, 243, 228, 184, 106, 67},
    {68, 73, 104, 153, 181, 188, 197, 203, 208, 213, 218, 227, 231, 233, 235, 243, 247, 247, 232, 203, 149, 92, 64},
    {65, 63, 86, 134, 167, 179, 190, 199, 204, 207, 212, 223, 226, 227, 230, 236, 238, 238, 219, 170, 116, 78, 59},
    {62, 56, 70, 103, 137, 166, 184, 193, 197, 202, 206, 212, 217, 219, 221, 221, 224, 223, 190, 125, 80, 61, 52},
    {58, 54, 63, 79, 105, 141, 166, 179, 190, 195, 197, 201, 206, 209, 206, 202, 197, 188, 154, 96, 65, 54, 51},
    {57, 51, 54, 63, 77, 104, 134, 158, 172, 178, 184, 193, 197, 196, 191, 175, 156, 136, 104, 77, 64, 58, 58},
    {54, 51, 53, 57, 64, 77, 97, 125, 145, 153, 165, 169, 173, 174, 166, 141, 113, 90, 72, 63, 58, 60, 59},
    {55, 56, 55, 57, 59, 63, 71, 82, 94, 118, 131, 138, 138, 139, 126, 100, 82, 68, 62, 56, 57, 63, 60},
    {59, 54, 56, 58, 56, 57, 57, 58, 67, 85, 95, 98, 101, 99, 90, 73, 65, 58, 59, 56, 56, 61, 58},
    {57, 57, 58, 55, 57, 59, 57, 59, 63, 71, 74, 73, 72, 72, 67, 62, 60, 60, 58, 56, 57, 60, 59},
};

int grey_cell_pattern2[CELL_PATTERN_SIZE][CELL_PATTERN_SIZE] = {
    {18, 13, 10, 8, 10, 11, 15, 16, 9, 12, 19, 19, 20, 15, 16, 15, 18, 16, 17, 14, 13, 15, 23},
    {15, 15, 12, 10, 10, 14, 16, 15, 13, 12, 18, 22, 18, 16, 15, 15, 14, 18, 15, 14, 14, 15, 21},
    {16, 16, 15, 13, 12, 16, 13, 13, 12, 13, 18, 17, 16, 12, 14, 17, 15, 16, 15, 17, 13, 15, 22},
    {17, 17, 12, 14, 13, 13, 15, 14, 12, 15, 18, 19, 13, 14, 16, 15, 14, 18, 18, 18, 18, 18, 21},
    {14, 13, 14, 13, 11, 13, 12, 13, 12, 12, 17, 16, 12, 14, 16, 17, 17, 18, 20, 21, 22, 25, 20},
    {13, 14, 13, 16, 13, 12, 10, 11, 16, 15, 17, 16, 16, 15, 13, 15, 17, 18, 17, 20, 20, 24, 23},
    {16, 14, 16, 17, 13, 9, 12, 12, 16, 18, 21, 20, 17, 14, 12, 13, 19, 19, 21, 20, 20, 19, 21},
    {19, 18, 18, 16, 13, 10, 11, 12, 15, 17, 22, 22, 15, 14, 12, 13, 19, 21, 20, 21, 18, 17, 18},
    {15, 17, 16, 16, 15, 13, 10, 7, 10, 15, 26, 23, 16, 13, 14, 13, 18, 18, 20, 16, 15, 14, 16},
    {16, 16, 17, 15, 13, 13, 10, 9, 11, 13, 20, 23, 17, 15, 17, 14, 18, 17, 17, 15, 15, 12, 16},
    {16, 18, 15, 17, 15, 15, 17, 17, 13, 16, 20, 20, 13, 14, 22, 20, 20, 19, 18, 17, 12, 13, 17},
    {15, 16, 17, 13, 16, 17, 18, 18, 17, 18, 20, 18, 13, 14, 22, 22, 18, 17, 15, 15, 13, 12, 19},
    {15, 14, 13, 12, 14, 14, 18, 18, 20, 19, 22, 21, 15, 14, 17, 20, 13, 10, 10, 11, 14, 17, 18},
    {13, 11, 14, 15, 13, 13, 13, 15, 21, 21, 22, 19, 16, 14, 17, 15, 12, 11, 10, 13, 15, 19, 15},
    {11, 12, 15, 16, 15, 15, 13, 18, 15, 18, 14, 16, 13, 13, 13, 14, 14, 18, 13, 14, 17, 16, 16},
    {8, 9, 15, 18, 15, 17, 15, 15, 16, 16, 17, 15, 14, 13, 15, 14, 19, 17, 15, 15, 17, 17, 15},
    {10, 13, 17, 19, 13, 14, 18, 19, 13, 16, 21, 22, 15, 18, 24, 22, 18, 16, 17, 17, 20, 20, 15},
    {13, 12, 16, 19, 12, 14, 19, 18, 15, 15, 22, 19, 18, 19, 22, 22, 16, 13, 17, 19, 22, 23, 17},
    {14, 13, 16, 15, 17, 16, 22, 20, 19, 16, 19, 17, 17, 17, 18, 16, 13, 13, 15, 19, 22, 25, 21},
    {19, 15, 16, 15, 16, 17, 18, 17, 19, 18, 17, 19, 19, 17, 16, 13, 11, 14, 16, 21, 21, 20, 23},
    {19, 20, 21, 22, 19, 20, 14, 12, 14, 14, 15, 17, 16, 16, 16, 13, 14, 12, 21, 19, 17, 16, 22},
    {22, 21, 20, 17, 19, 15, 13, 13, 13, 16, 15, 17, 15, 18, 16, 13, 14, 14, 16, 18, 17, 16, 20},
    {19, 19, 13, 12, 14, 16, 16, 16, 19, 19, 16, 14, 17, 15, 17, 13, 12, 12, 15, 15, 13, 15, 18}};

int grey_cell_pattern3[CELL_PATTERN_SIZE][CELL_PATTERN_SIZE] = {
    {104, 106, 111, 117, 121, 112, 99, 84, 72, 62, 53, 46, 40, 37, 38, 39, 37, 37, 38, 39, 36, 36, 36},
    {119, 125, 129, 133, 140, 133, 115, 100, 86, 72, 60, 51, 44, 40, 42, 39, 38, 38, 35, 38, 38, 39, 37},
    {135, 141, 149, 153, 153, 148, 131, 117, 98, 84, 73, 59, 50, 42, 44, 42, 38, 35, 33, 35, 40, 41, 40},
    {144, 152, 161, 167, 168, 164, 146, 130, 114, 96, 82, 68, 56, 47, 47, 42, 39, 37, 35, 36, 39, 40, 39},
    {148, 154, 167, 177, 183, 177, 161, 142, 124, 105, 90, 76, 64, 53, 43, 39, 37, 39, 39, 39, 39, 38, 39},
    {145, 153, 171, 182, 191, 186, 170, 153, 132, 115, 99, 83, 67, 57, 43, 38, 39, 39, 39, 38, 37, 40, 39},
    {142, 154, 172, 186, 190, 190, 174, 159, 136, 119, 103, 89, 72, 56, 47, 39, 38, 39, 40, 39, 39, 37, 38},
    {139, 150, 167, 180, 187, 184, 174, 160, 141, 121, 107, 90, 75, 57, 47, 39, 39, 38, 39, 40, 37, 36, 34},
    {134, 145, 162, 172, 175, 172, 168, 159, 140, 125, 110, 93, 74, 61, 50, 44, 37, 37, 37, 35, 37, 36, 36},
    {132, 141, 155, 167, 168, 166, 163, 153, 137, 122, 108, 91, 75, 61, 54, 43, 38, 37, 36, 36, 36, 35, 37},
    {131, 140, 154, 163, 167, 163, 156, 143, 128, 115, 107, 93, 74, 64, 51, 45, 39, 37, 38, 39, 34, 36, 38},
    {128, 136, 152, 161, 164, 161, 155, 138, 122, 110, 102, 89, 74, 62, 55, 45, 38, 34, 39, 38, 39, 40, 39},
    {127, 136, 148, 157, 160, 156, 148, 135, 121, 105, 95, 84, 72, 62, 54, 46, 38, 36, 35, 34, 40, 38, 40},
    {125, 132, 145, 154, 159, 156, 143, 136, 121, 106, 93, 80, 70, 59, 53, 44, 40, 37, 35, 36, 39, 40, 39},
    {119, 125, 137, 151, 159, 160, 149, 139, 124, 109, 94, 80, 68, 57, 47, 38, 39, 35, 32, 32, 36, 38, 39},
    {112, 119, 135, 146, 155, 159, 149, 137, 121, 109, 91, 76, 66, 54, 44, 40, 37, 33, 32, 31, 34, 37, 39},
    {108, 114, 130, 145, 154, 154, 145, 132, 115, 101, 84, 69, 57, 50, 45, 38, 39, 37, 40, 38, 37, 37, 37},
    {102, 108, 124, 138, 145, 146, 136, 127, 109, 96, 79, 66, 55, 46, 43, 38, 35, 36, 40, 40, 38, 36, 36},
    {95, 102, 114, 125, 130, 132, 126, 116, 102, 88, 74, 61, 51, 44, 40, 39, 34, 34, 37, 40, 38, 40, 43},
    {86, 92, 100, 110, 113, 115, 114, 104, 94, 79, 68, 58, 51, 42, 39, 38, 33, 35, 36, 36, 39, 40, 42},
    {75, 78, 87, 94, 96, 97, 95, 88, 80, 70, 59, 52, 45, 41, 39, 38, 36, 34, 35, 33, 39, 38, 39},
    {64, 68, 76, 81, 80, 79, 80, 75, 67, 60, 55, 47, 40, 39, 39, 42, 38, 34, 30, 31, 36, 39, 39},
    {59, 61, 65, 66, 71, 70, 66, 62, 58, 52, 48, 45, 39, 36, 39, 40, 38, 37, 34, 32, 33, 39, 41},
};

#define ARRAY_CELL_SIZE 23

#define NCCRequirement 0.85

double get_templateLength(double templateLength)
{
    for (int cellx = 0; cellx < ARRAY_CELL_SIZE; cellx++)
    {
        for (int celly = 0; celly < ARRAY_CELL_SIZE; celly++)
        {
            templateLength += (grey_cell_pattern3[cellx][celly]) ^ 2;
        }
    }

    templateLength = sqrt(templateLength);

    return templateLength;
}

double use_pattern(unsigned char image[BMP_WIDTH][BMP_HEIGTH], int x, int y)
{
    unsigned int sum = 0;
    double patchLength = 0;

    for (int cellx = 0; cellx < ARRAY_CELL_SIZE; cellx++)
    {
        for (int celly = 0; celly < ARRAY_CELL_SIZE; celly++)
        {
            if (inside_bounds(x + cellx, y + celly))
            {
                sum += image[x + cellx][y + celly] * grey_cell_pattern[cellx][celly];
                patchLength += (image[x + cellx][y + celly]) ^ 2;
            }
        }
    }

    patchLength = sqrt(patchLength);

    //printf("Template length: %d / %f * %f \n \n \n \n \n",sum,patchLength,templateLength);

    double NCC1 = (sum / (23 * 23)) / (patchLength * templateLength1);
    double NCC2 = (sum / (23 * 23)) / (patchLength * templateLength2);
    double NCC3 = (sum / (23 * 23)) / (patchLength * templateLength3);

    if (NCC1 >= NCC2 && NCC1 >= NCC3)
    {
        return NCC1;
    }
    else if (NCC2 >= NCC1 && NCC2 >= NCC3)
    {
        return NCC2;
    }
    else
    {
        return NCC3;
    }
}

void remove_pattern_cell(int x, int y, unsigned char image[BMP_WIDTH][BMP_HEIGTH])
{
    for (char width = 0; width < ARRAY_CELL_SIZE; width++)
    {
        for (char height = 0; height < ARRAY_CELL_SIZE; height++)
        {
            if (inside_bounds(x + width, y + height))
            {
                image[x + width][y + height] = 0;
            }
        }
    }
}

void pattern_search(unsigned char image[BMP_WIDTH][BMP_HEIGTH])
{
    //double templateLength = 0;
    //templateLength = get_templateLength(templateLength);
    //printf("%f",templateLength);

    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            if (use_pattern(image, x, y) >= NCCRequirement && not_too_close(x, y) == 1)
            {
                countedCells++;
                mark_cell(x + 12, y + 12);
                remove_pattern_cell(x, y, image);
            }
        }
    }
}
*/
/*
    Main
*/

int main(int nPassedArguments, char **args)
{
    if (nPassedArguments != 4)
    {
        fprintf(stderr, "Needs 4 arguments: %s <input file path> <output file path> erosion = e, pattern = p\n", args[0]);
        exit(1);
    }

    //  read file
    read_bitmap(args[1], input_image);

    clock_t start, end;
    double cpu_time_used;

    start = clock();

    //outputEqualsInput();

    //  greyscale image
    unsigned char grey_image[BMP_WIDTH][BMP_HEIGTH];
    grey_scale_image(grey_image, input_image);

    // Pattern or erosion

    if (strcmp(args[3], "p") == 0)
    {
        pattern_search(grey_image);
    }
    else
    {
        //pointer - could be used to optimize
        //unsigned char(* aPtr)[BMP_WIDTH][BMP_HEIGTH];
        //aPtr = &grey_image;

        binary_threshold(grey_image);

        erosion(grey_image);
    }

    end = clock();

    //image_to_3d(grey_image);

    //  create bmp file
    write_bitmap(input_image, args[2]);

    print_coordinates();

    printf("Cells counted: %d \n", countedCells);

    printf("Total erosions: %d \n", totalErosions);

    cpu_time_used = end - start;
    printf("Total time: %f ms\n", cpu_time_used * 1000.0 / CLOCKS_PER_SEC);

    return 0;
}
