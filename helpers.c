#include "helpers.h"
#include <math.h>
#include <stdbool.h>

typedef struct
{
    int sumBlue;
    int sumGreen;
    int sumRed;
    int count;
}
sumRGB;

void clear(sumRGB *average);
bool isOnMatrix(int i, int j, int height, int width);
void copyImage(int height, int width, RGBTRIPLE image[height][width], RGBTRIPLE tmp_image[height][width]);
int  roundPixel(double number);
double combineKernels(int Gx, int Gy);
double makeSepia(char flag, BYTE red, BYTE green, BYTE blue);

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    // Sets the average variable and the sum
    float average = 0;
    int sum = 0;

    // Loops to the intire array image
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Takes the sum to do the average
            sum = image[i][j].rgbtBlue + image[i][j].rgbtGreen + image[i][j].rgbtRed;
            average = (float)sum / 3;

            // Reassign the rounded average values to the corresponding pixels
            image[i][j].rgbtBlue  = round(average);
            image[i][j].rgbtGreen = round(average);
            image[i][j].rgbtRed   = round(average);
        }
    }
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    // This is a temporary array of pixels that stores the pixels that are going reflected
    int tmp[3];

    for (int i = 0; i < height; i++)
    {
        for (int j = 0, k = width - 1, n = width / 2; j < n; j++, k--)
        {
            // Save the pixels that will be swapped
            tmp[0] = image[i][j].rgbtBlue;
            tmp[1] = image[i][j].rgbtGreen;
            tmp[2] = image[i][j].rgbtRed;

            // Swaps the last with the first pixels
            image[i][j].rgbtBlue  = image[i][k].rgbtBlue;
            image[i][j].rgbtGreen = image[i][k].rgbtGreen;
            image[i][j].rgbtRed   = image[i][k].rgbtRed;

            // Swaps back the first pixels to the last positions
            image[i][k].rgbtBlue  = tmp[0];
            image[i][k].rgbtGreen = tmp[1];
            image[i][k].rgbtRed   = tmp[2];
        }
    }

    return;
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    // Creates a variable that can handle the RGB blurring
    sumRGB average;
    clear(&average);

    // Initializes the line and column values (line stands for the i and column the j)
    int line = 0, column = 0;

    // Creates a temporary copy of the image that, after updated, copies back to the original image
    RGBTRIPLE tmp_image[height][width];

    // I wanna loop throught the entire function until I get to the end of the image
    // wich is at line height - 1
    while (line < height)
    {
        for (int i = line - 1; i <= line + 1; i++)
        {
            for (int j = column - 1; j <= column + 1; j++)
            {
                if (isOnMatrix(i, j, height, width))
                {
                    average.sumBlue += image[i][j].rgbtBlue;
                    average.sumGreen += image[i][j].rgbtGreen;
                    average.sumRed += image[i][j].rgbtRed;

                    average.count++;
                }
            }
        }

        // Blurs the image and copies its pixels to a temporary image
        tmp_image[line][column].rgbtBlue  = round((double)average.sumBlue / average.count);
        tmp_image[line][column].rgbtGreen = round((double)average.sumGreen / average.count);
        tmp_image[line][column].rgbtRed   = round((double)average.sumRed / average.count);

        // Line and colmun need to update in a different rate. If colmun == width - 1, then line
        // should get +1
        if (column == width - 1)
        {
            line++;
            column = 0;
        }
        else
            column++;

        clear(&average);
    }

    // Copies the temporary image into the real image
    copyImage(height, width, image, tmp_image);

    return;
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    // Kernel in the X direction
    const int Gx [3][3] =
    {
        {-1,0,1},
        {-2,0,2},
        {-1,0,1},
    };

    // Kernel in the Y direction
    const int Gy [3][3] =
    {
        {-1,-2,-1},
        {0,0,0},
        {1,2,1},
    };

    // Initializes the matrix i and j so I can use it to flow on the image
    // Its set to 1, so I can firstly ignore the border cases
    int image_column = 0, image_line = 0;

    // Initializes the sums if Gx and Gy variables and sets them all to 0
    sumRGB sumGx, sumGy;

    clear(&sumGx);
    clear(&sumGy);

    // Creates a new and temporary image, so I can save the new RGB values into a new image
    // without need to modify the original one
    RGBTRIPLE tmp_image[height][width];

    while (image_line < height)
    {
        for (int i = image_line - 1, kernel_line = 0; i <= image_line + 1; i++, kernel_line++)
        {
            for (int j = image_column - 1, kernel_column = 0; j <= image_column + 1; j++, kernel_column++)
            {
                if (isOnMatrix(i, j, height, width))
                {
                    // Gets the RGB pixels value and multiplies it by the Gx corresponded values
                    sumGx.sumBlue  += image[i][j].rgbtBlue  * Gx[kernel_line][kernel_column];
                    sumGx.sumGreen += image[i][j].rgbtGreen * Gx[kernel_line][kernel_column];
                    sumGx.sumRed   += image[i][j].rgbtRed   * Gx[kernel_line][kernel_column];

                    // Gets the RGB pixels value and multiplies it by the Gy corresponded values
                    sumGy.sumBlue  += image[i][j].rgbtBlue  * Gy[kernel_line][kernel_column];
                    sumGy.sumGreen += image[i][j].rgbtGreen * Gy[kernel_line][kernel_column];
                    sumGy.sumRed   += image[i][j].rgbtRed   * Gy[kernel_line][kernel_column];
                }
                else
                {
                    // Considers a hole solid black border in the image, for both Gx and Gy
                    sumGx.sumBlue  += 0;
                    sumGx.sumGreen += 0;
                    sumGx.sumRed   += 0;

                    sumGy.sumBlue  += 0;
                    sumGy.sumGreen += 0;
                    sumGy.sumRed   += 0;
                 }
            }
        }

        // Combines the 2 kernels (Gx and Gy) into a unic one, so I can detect edges in both horizontal and vertical directions
        tmp_image[image_line][image_column].rgbtBlue  = roundPixel(combineKernels(sumGx.sumBlue, sumGy.sumBlue));
        tmp_image[image_line][image_column].rgbtGreen = roundPixel(combineKernels(sumGx.sumGreen, sumGy.sumGreen));
        tmp_image[image_line][image_column].rgbtRed   = roundPixel(combineKernels(sumGx.sumRed, sumGy.sumRed));

        if (image_column == width - 1)
        {
            image_column = 0;
            image_line++;
        }
        else
            image_column++;

        clear(&sumGx);
        clear(&sumGy);
    }

    // Copies the temporary image into the real image
    copyImage(height, width, image, tmp_image);

    return;
}

void sepia(int height, int width, RGBTRIPLE image[height][width])
{
    // Creates a temporary image
    RGBTRIPLE tmp_image[height][width];
    
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Changes the image values into the tmp_image to get a sepia style to all RGB colors
            tmp_image[i][j].rgbtBlue  = roundPixel(makeSepia('b', image[i][j].rgbtRed, image[i][j].rgbtGreen, image[i][j].rgbtBlue));
            tmp_image[i][j].rgbtGreen = roundPixel(makeSepia('g', image[i][j].rgbtRed, image[i][j].rgbtGreen, image[i][j].rgbtBlue));
            tmp_image[i][j].rgbtRed   = roundPixel(makeSepia('r', image[i][j].rgbtRed, image[i][j].rgbtGreen, image[i][j].rgbtBlue));
        }
    }
    
    copyImage(height, width, image, tmp_image);
    
    return;
}

void clear(sumRGB *average)
{
    average->sumBlue = 0;
    average->sumGreen = 0;
    average->sumRed = 0;
    average->count = 0;

    return;
}

bool isOnMatrix(int i, int j, int height, int width)
{
    if (i >= 0 && j >= 0 && i < height && j < width)
        return true;
    else
        return false;
}

void copyImage(int height, int width, RGBTRIPLE image[height][width], RGBTRIPLE tmp_image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            image[i][j].rgbtBlue = tmp_image[i][j].rgbtBlue;
            image[i][j].rgbtGreen = tmp_image[i][j].rgbtGreen;
            image[i][j].rgbtRed = tmp_image[i][j].rgbtRed;
        }
    }

    return;
}

int roundPixel(double number)
{
    if (number > 255)
        return 255;
    else if (number < 0)
        return 0;
    else
        return round((double)number);
}

double combineKernels(int Gx, int Gy)
{
    int powGx = pow(Gx, 2);
    int powGy = pow(Gy, 2);

    return sqrt(powGx + powGy);
}

double makeSepia(char flag, BYTE red, BYTE green, BYTE blue)
{
    if (flag == 'r')
    {
        return .393 * red + .769 * green + .189 * blue;
    }
    else if (flag == 'g')
    {
        return .349 * red + .686 * green + .168 * blue;
    }
    else if (flag == 'b')
    {
        return .272 * red + .534 * green + .131 * blue;
    }
    else return -1;
}