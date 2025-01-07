#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// Structure representing an image and its properties
typedef struct{
	int **pixels;   // 2D array for the pixels
	int width;      // (rows)
	int length;     // (columns)
	int max_value;
	int pixel_depth; // Channels (1 for grayscale, 3 for RGB)
	int x1, x2, y1, y2; // Selected part of the image
} image_t;

// Free memory allocated for the image pixel array
void free_image(image_t *photo)
{
	if (photo && photo->pixels) {
		for (int i = 0; i < photo->width; i++) {
			free(photo->pixels[i]);
		}
		free(photo->pixels);
		photo->pixels = NULL;
	}
}

// Swap two integers
void swap(int *x, int *y)
{
	int z = 0;
	z = *x;
	*x = *y;
	*y = z;
}

// Ensure two integers are sorted in ascending order
void sort_int(int *x, int *y)
{
	if (*x > *y) {
		swap(x, y);
	}
}

// Default values
void initialize_struct(image_t *photo)
{
	photo->pixels = NULL;
	photo->width = 0;
	photo->length = 0;
	photo->max_value = 0;
	photo->pixel_depth = 0;
	photo->x1 = 0;
	photo->y1 = 0;
	photo->x2 = 0;
	photo->y2 = 0;
}

// Get the kernel matrix for a specified filter
void get_kernel(double kernel[3][3], char *filter)
{
	double filters[5][3][3] = {
		{{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}}, // EDGE
		{{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}}, // SHARPEN
		{{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}}, // BLUR
		{{1.0, 2.0, 1.0}, {2.0, 4.0, 2.0}, {1.0, 2.0, 1.0}}, // GAUSS_B
		{{0, 0, 0}, {0, 1, 0}, {0, 0, 0}} // IDENTITY
	};
	int selection = 4; // Default = IDENTITY
	if (!strcmp(filter, "EDGE\n")) {
		selection = 0;
	}
	if (!strcmp(filter, "SHARPEN\n")) {
		selection = 1;
	}
	if (!strcmp(filter, "BLUR\n")) {
		selection = 2;
	}
	if (!strcmp(filter, "GAUSSIAN_BLUR\n")) {
		selection = 3;
	}
	if (selection == 4) {
		printf("APPLY parameter invalid\n");
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			kernel[i][j] = filters[selection][i][j];
		}
	}
	// Remove the ending '\n' for correct printing
	filter[strlen(filter) - 1] = '\0';

	if (selection == 4) {
		return; // We exit the function after
				// populating the matrix
	}

	printf("APPLY %s done\n", filter);
}

// Apply a specified filter to the image
void handle_apply(image_t *photo, char *argument)
{
	if (!photo->length) {
		printf("No image loaded\n");
		return;
	}
	if (!argument) {
		printf("Invalid command\n");
		return;
	}
	if (photo->pixel_depth == 1) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	double kernel[3][3];
	int **temp;

	get_kernel(kernel, argument);
	temp = (int **)calloc(photo->width, sizeof(int *));
	if (!temp) {
		perror("Failed to allocate row pointers");
		return;
	}
	int value = 1; // without "int value" there are floating point errors
	if (!strcmp(argument, "BLUR")) {
		value = 9;
	}
	if (!strcmp(argument, "GAUSSIAN_BLUR")) {
		value = 16;
	} // Allocate memory for each row
	for (int i = 0; i < photo->width; i++) {
		(temp)[i] = (int *)calloc(photo->length *
		photo->pixel_depth, sizeof(int));
		if ((temp)[i] == NULL) {
			perror("Failed to allocate row");
			// Free any previously allocated rows
			for (int j = 0; j < i; j++) {
				free((temp)[j]);
			}
			free(temp);
			return;
		}
	}
	double sum = 0.0; // Copy the pixels matrix to our temporary
	for (int i = 0 ; i < photo->width; i++) {
		for (int j = 0; j < photo->length * photo->pixel_depth; j++) {
			temp[i][j] = photo->pixels[i][j];
		}
	} // Calculate new pixel values based on original matrix
	for (int i = photo->y1; i < photo->y2; i++) {
		for (int j = photo->x1 * photo->pixel_depth;
			 j < photo->x2 * photo->pixel_depth; j++) {
			sum = 0; // For border
			if (i == 0 || j < 3 || j >= (photo->length *
			    photo->pixel_depth - 3) || i == photo->width - 1) {
				temp[i][j] = photo->pixels[i][j];
			} else { // Calculates new value
				sum = sum + photo->pixels[i - 1][j - 3] * kernel[0][0];
				sum = sum + photo->pixels[i - 1][j] * kernel[0][1];
				sum = sum + photo->pixels[i - 1][j + 3] * kernel[0][2];
				sum = sum + photo->pixels[i][j - 3] * kernel[1][0];
				sum = sum + photo->pixels[i][j] * kernel[1][1];
				sum = sum + photo->pixels[i][j + 3] * kernel[1][2];
				sum = sum + photo->pixels[i + 1][j - 3] * kernel[2][0];
				sum = sum + photo->pixels[i + 1][j] * kernel[2][1];
				sum = sum + photo->pixels[i + 1][j + 3] * kernel[2][2];
				sum /= value; // Clamps values
				if (sum < 0) {
					sum = 0;
				}
				if (sum > 255) {
					sum = 255;
				} // Rounds the float and puts the value in the matrix
				temp[i][j] = (int)(round(sum));
			}
		}
	}
	free_image(photo); // Free the memory
	photo->pixels = temp; // Change the pointer to the new matrix
}

// Appoints values to the variables in the struct that control
// what part of the image is selected
void handle_select(image_t *photo, char *argument)
{
	int x1, x2, y1, y2; // placeholders
	if (!photo->length) {
		printf("No image loaded\n");
		return;
	}
	// removes junk from the end of the input
	while (argument[strlen(argument) - 1] == '\n' ||
		   argument[strlen(argument) - 1] == ' ') {
		argument[strlen(argument) - 1] = '\0';
	}
	if (!strcmp(argument, "ALL")) {
		photo->x1 = 0;
		photo->x2 = photo->length;
		photo->y1 = 0;
		photo->y2 = photo->width;
		printf(("Selected ALL\n"));
		return;
	}
	if (sscanf(argument, "%d %d %d %d", &x1, &y1, &x2, &y2) != 4) {
		printf("Invalid command\n");
		return;
	}
	// makes sure they are in ascending order
	sort_int(&x1, &x2);
	sort_int(&y1, &y2);
	// checks for invalid data
	if (x1 == x2 || y1 == y2 || x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 ||
		x1 > photo->length || x2 > photo->length ||
		y1 > photo->width || y2 > photo->width) {
		printf("Invalid set of coordinates\n");
		return;
	}
	photo->x1 = x1;
	photo->y1 = y1;
	photo->x2 = x2;
	photo->y2 = y2;
	printf("Selected %d %d %d %d\n", photo->x1,
		   photo->y1, photo->x2, photo->y2);
}

// Cuts the photo based on the selection
// The cut is then considered a new photo
void handle_crop(image_t *photo, char *argument)
{

	if (argument) {
		return;
	}

	if (!photo->length || !photo->width) {
		printf("No image loaded\n");
		return;
	}

	// Calculate the dimensions of the cropped area
	int new_width = photo->y2 - photo->y1;
	int new_length = photo->x2 - photo->x1;

	// Allocate memory for the new cropped matrix
	int **new_pixels = (int **)calloc(new_width, sizeof(int *));
	if (!new_pixels) {
		perror("Failed to allocate memory for cropped matrix");
		return;
	}

	for (int i = 0; i < new_width; i++) {
		new_pixels[i] = (int *)calloc(new_length *
		photo->pixel_depth, sizeof(int));
		if (!new_pixels[i]) {
			perror("Failed to allocate memory for cropped matrix row");
			// Free any previously allocated rows
			for (int j = 0; j < i; j++) {
				free(new_pixels[j]);
			}
			free(new_pixels);
			return;
		}
	}

	// Copy the selected portion of the original to the new matrix
	int a = 0, b = 0;
	for (int i = photo->y1; i < photo->y2; i++) {
		for (int j = photo->x1 * photo->pixel_depth; j < photo->x2
			 * photo->pixel_depth; j++) {
			new_pixels[a][b++] = photo->pixels[i][j];
		}
		a++;
		b = 0;
	}

	// Free the original
	free_image(photo);
	// Update the photo struct
	photo->pixels = new_pixels;
	photo->length = new_length;
	photo->width = new_width;
	photo->x1 = 0;
	photo->y1 = 0;
	photo->x2 = new_length;
	photo->y2 = new_width;
	printf("Image cropped\n");
}

// Saves the image it holds to a specified file (in binary)
void save_binary_image(image_t *photo, char *file_path)
{
	if (!photo->length) {
		printf("No image loaded\n");
		return;
	}

	// removes junk at end of input
	while (file_path[strlen(file_path) - 1] == '\n' ||
		   file_path[strlen(file_path) - 1] == ' ') {
		file_path[strlen(file_path) - 1] = '\0';
	}

	FILE *file = fopen(file_path, "wb"); // Open the file in binary mode

	if (!file) {
		perror("Failed to open file");
		return;
	}
	// Determine the format (P5 for grayscale, P6 for RGB)
	char *format = NULL;
	if (photo->pixel_depth == 1) {
		format = "P5";
	} else
		format = "P6";

	// Write the header
	fprintf(file, "%s\n%d %d\n%d\n", format, photo->length,
			photo->width, photo->max_value);

	// Write pixel data
	for (int i = 0; i < photo->width; i++) {
		for (int j = 0; j < photo->length * photo->pixel_depth; j++) {
			unsigned char pixel_value = photo->pixels[i][j];
			fwrite(&pixel_value, sizeof(unsigned char), 1, file);
		}
	}

	fclose(file);
	printf("Saved %s\n", file_path);
}

// Saves the image it holds to a specified file (in plain text)
void handle_save(image_t *photo, char *argument)
{
	// For ascii the implementation is here
	// For binary in a seperate function we call here
	// ( save_binary_image(...) )
	if (!photo->length) {
		printf("No image loaded\n");
		return;
	}
	if (argument[strlen(argument) - 3] == 'i') { // (ends in "ascii")
		char *file_path = strtok(argument, " ");
		FILE *file = fopen(file_path, "w");
		if (!file) {
			perror("Failed to open file");
			return;
		}
		if (photo->pixel_depth == 1) {
			fprintf(file, "P2");
		} else {
			fprintf(file, "P3");
		}
		fprintf(file, "\n%d %d\n%d\n", photo->length,
				photo->width, photo->max_value);
		for (int i = 0; i < photo->width; i++) {
			for (int j = 0; j < photo->length * photo->pixel_depth; j++) {
				fprintf(file, "%d ", photo->pixels[i][j]);
			}
			fprintf(file, "\n");
		}
		printf("Saved %s\n", file_path);
		fclose(file);
	} else {
		// calls for the binary save function
		save_binary_image(photo, argument);
	}
}

// Frees all memory and stops the program
void handle_exit(image_t *photo, char *argument)
{
	if (argument) {
		return;
	}
	if (!photo->length) {
		printf("No image loaded\n");
	} else {
		free_image(photo);
	}
	exit(0);
}

// argument should look like "HISTOGRAM x y"
// calculates the value of y bins and only PRINTS them
// with x being the max value of "*" a bin can have
void handle_histogram(image_t *photo, char *argument)
{
	if (!photo->length) {
		printf("No image loaded\n");
		return;
	}
	if (!argument) {
		printf("Invalid command\n");
		return;
	}
	int x, y;
	int error;
	float cy;
	// tries to read more then normal to see if bad input from user
	if (sscanf(argument, "%d %d %d", &x, &y, &error) != 2) {
		printf("Invalid command\n");
		return;
	}
	cy = y;
	if (y == 0 || y == 1) {
		printf("Invalid set of parameters\n");
		return;
	}
	// checks if y is power of 2
	while (cy != 1) {
		cy /= 2.0;
		if (cy != (int)(cy)) {
			printf("Invalid set of parameters\n");
			return;
		}
	}
	if (photo->pixel_depth == 3) {
		printf("Black and white image needed\n");
		return;
	}
	int freq[256] = {0};
	int values[256] = {0};
	int group_size = 256 / y;
	int sum = 0, max_sum = 0, c_group_size = group_size;
	int it = 0;

	// freq : number of appearances of every pixel value
	for (int i = 0 ; i < photo->width; i++) {
		for (int j = 0; j < photo->length; j++) {
			freq[photo->pixels[i][j]]++;
		}
	}
	// values : values for every BIN
	// group_size * y = 256
	for (int i = 0; i < 256; i++) {
		sum = 0;
		while (c_group_size) {
			sum += freq[i++];
			c_group_size--;
		}
		values[it++] = sum;
		if (sum > max_sum) {
			max_sum = sum;
		}
		c_group_size = group_size;
		i--;
	}
	// calculates the number of stars using the given formula
	// current_value / max_value * x
	for (int i = 0; i < it; i++) {
		double stars_f = 1.0 * values[i] / max_sum * x;
		int stars = (int)(stars_f);
		printf("%d\t|\t", stars);
		while (stars) {
			printf("*");
			stars--;
		}
		printf("\n");
	}
}

// changes the pixel values for the WHOLE matrix based of a formula
// selection doesn't affect equalize
void handle_equalize(image_t *photo, char *argument)
{
	if (argument) {
		return;
	}
	if (!photo->length) {
		printf("No image loaded\n");
		return;
	}
	if (photo->pixel_depth == 3) {
		printf("Black and white image needed\n");
		return;
	}
	int freq[256] = {0};
	// calculates the frequency of all pixels
	for (int i  = 0; i < photo->width; i++) {
		for (int j = 0; j < photo->length; j++) {
			freq[photo->pixels[i][j]]++;
		}
	}
	// adds to every pixel frequency
	// the pixel frequencies of those less than it
	// ! from right to left to calculate correctly
	for (int i = 255; i >= 0; i--) {
		for (int j = 0; j < i; j++) {
			freq[i] += freq[j];
		}
	}
	// changes pixels based on the given formula
	// 255 / AREA * (sum of pixels up to [i])
	for (int i = 0; i < photo->width; i++) {
		for (int j = 0; j < photo->length; j++) {
			double new_value = 0;
			new_value = 255.0 / photo->width / photo->length *
			freq[photo->pixels[i][j]];
			// clamps
			if (new_value > 255) {
				new_value = 255;
			}
			// rounds before attributing it to the matrix
			photo->pixels[i][j] = (int)(round(new_value));
		}
	}
	printf("Equalize done\n");
}

// Handles the rotation for the whole matrix
// ! Changes the selection for 90/270 rotations
void rotate_full_matrix(image_t *photo, int degrees)
{
	// Normalize degrees to [0, 360)
	degrees = (degrees % 360 + 360) % 360;

	// checks if dimensions AND SELECTION swap during rotation
	int new_width = (degrees == 90 || degrees == 270) ?
	photo->length : photo->width;
	int new_length = (degrees == 90 || degrees == 270) ?
	photo->width : photo->length;
	(degrees == 90 || degrees == 270) ? swap(&photo->x2, &photo->y2) : 1;
	// Allocate a new matrix for the rotated image
	int **rotated_pixels = (int **)malloc(new_width * sizeof(int *));
	for (int i = 0; i < new_width; i++) {
		rotated_pixels[i] = (int *)malloc(new_length *
		photo->pixel_depth * sizeof(int));
	}
	if (degrees == 90) {
		for (int i = 0; i < photo->width; i++) {
			for (int j = 0; j < photo->length; j++) {
				for (int k = 0; k < photo->pixel_depth; k++) {
					rotated_pixels[j][(photo->width - 1 - i)
					* photo->pixel_depth + k] = photo->pixels[i]
					[j * photo->pixel_depth + k];
				}
			}
		}
	} else if (degrees == 180) {
		for (int i = 0; i < photo->width; i++) {
			for (int j = 0; j < photo->length; j++) {
				for (int k = 0; k < photo->pixel_depth; k++) {
					rotated_pixels[photo->width - 1 - i]
					[(photo->length - 1 - j) *
					photo->pixel_depth + k] =
					photo->pixels[i][j * photo->pixel_depth + k];
				}
			}
		}
	} else if (degrees == 270) {
		for (int i = 0; i < photo->width; i++) {
			for (int j = 0; j < photo->length; j++) {
				for (int k = 0; k < photo->pixel_depth; k++) {
					rotated_pixels[photo->length - 1 - j]
					[i * photo->pixel_depth + k] =
					photo->pixels[i][j * photo->pixel_depth + k];
				}
			}
		}
	}

	// Free old matrix
	for (int i = 0; i < photo->width; i++) {
		free(photo->pixels[i]);
	}
	free(photo->pixels);

	// Update matrix
	photo->pixels = rotated_pixels;
	photo->width = new_width;
	photo->length = new_length;
}

// Rotates only the square selection of the matrix
void rotate_selection(image_t *photo, int degrees)
{
	int cols = photo->x2 - photo->x1; // Number of columns (x-axis range)
	int rows = photo->y2 - photo->y1; // Number of rows (y-axis range)
	// Fix negative rotations
	if (degrees == -90) {
		degrees = 270;
	} else if (degrees == -270) {
		degrees = 90;
	} else if (degrees == -180) {
		degrees = 180;
	}

	// Temporary matrix for the rotated submatrix
	int **temp = (int **)malloc(cols * sizeof(int *));
	for (int i = 0; i < cols; i++) {
		temp[i] = (int *)malloc(rows * photo->pixel_depth * sizeof(int));
	}

	if (degrees == 90) {
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				for (int k = 0; k < photo->pixel_depth; k++) {
					temp[j][(rows - i - 1) * photo->pixel_depth + k] =
					photo->pixels[photo->y1 + i][(photo->x1 + j)
					* photo->pixel_depth + k];
				}
			}
		}
	} else if (degrees == 180) {
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				for (int k = 0; k < photo->pixel_depth; k++) {
					temp[rows - i - 1][(cols - j - 1) *
					photo->pixel_depth + k] =
					photo->pixels[photo->y1 + i][(photo->x1 + j)
					* photo->pixel_depth + k];
				}
			}
		}
	} else if (degrees == 270) {
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				for (int k = 0; k < photo->pixel_depth; k++) {
					temp[cols - j - 1][i * photo->pixel_depth + k] =
					photo->pixels[photo->y1 + i][(photo->x1 + j)
					* photo->pixel_depth + k];
				}
			}
		}
	} else {
		printf("WRONG DEGREE\n");
	}

	// Copy rotated submatrix back to the original matrix
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			for (int k = 0; k < photo->pixel_depth; k++) {
				photo->pixels[photo->y1 + i][(photo->x1 + j)
				* photo->pixel_depth + k] =
				temp[i][j * photo->pixel_depth + k];
			}
		}
	}
	// Free the temporary matrix
	for (int i = 0; i < cols; i++) {
		free(temp[i]);
	}
	free(temp);
}

// Used to decide what function should be called
// to handle the rotation
void handle_rotate(image_t *photo, char *argument)
{
	int degrees = atoi(argument);
	if (!photo->length) {
		printf("No image loaded\n");
		return;
	}
	if (degrees % 90) {
		printf("Unsupported rotation angle\n");
		return;
	}
	if (!(degrees % 360)) {
		printf("Rotated %d\n", degrees);
		return;
	}
	// checks what function is needed based on selection
	if (!photo->x1 && !photo->y1 && photo->x2 == photo->length &&
		photo->y2 == photo->width && photo->width != photo->length) {
		rotate_full_matrix(photo, degrees);
	} else {
		if (photo->x2 - photo->x1 != photo->y2 - photo->y1) {
			printf("Invalid set of coordinates");
			return;
		}
		rotate_selection(photo, degrees);
	}
	printf("Rotated %d\n", degrees);
}

// Takes the current loaded file
// and replaces it with the new one, read in plain text
void text_file_image(image_t *photo, char *file_name)
{

	free_image(photo); // frees the old image
	FILE *file = fopen(file_name, "r");
	fseek(file, 2, SEEK_SET); // skips format
	fscanf(file, " %d %d %d", &photo->length, &photo->width, &photo->max_value);

	photo->pixels = (int **)calloc(photo->width, sizeof(int *));
	if (!photo->pixels) {
		perror("Failed to allocate row pointers");
		fclose(file);
		return;
	}

	for (int i = 0; i < photo->width; i++) {
		(photo->pixels)[i] = (int *)calloc(photo->length *
							 photo->pixel_depth, sizeof(int));
		if (!photo->pixels[i]) {
			perror("Failed to allocate row");
			// Free any previously allocated rows
			for (int j = 0; j < i; j++) {
				free((photo->pixels)[j]);
			}
			free(photo->pixels);
			*photo->pixels = NULL;
			fclose(file);
			return;
		}
	}
	// reads new matrix
	for (int i = 0; i < photo->width; i++) {
		for (int j = 0; j < photo->length * photo->pixel_depth; j++) {
			fscanf(file, "%d", &photo->pixels[i][j]);
		}
	}
	printf("Loaded %s\n", file_name);
	// changes default selection
	photo->x1 = 0;
	photo->x2 = photo->length;
	photo->y1 = 0;
	photo->y2 = photo->width;
	fclose(file);
}

// Same as text_file_image but for binary files
void binary_file_image(image_t *photo, char *file_name)
{
	free_image(photo);
	FILE *file = fopen(file_name, "rb");

	if (!file) {
		perror("Failed to open file");
		return;
	}
	fseek(file, 2, SEEK_SET); // skips format
	// reads header
	fscanf(file, " %d %d %d\n", &photo->length,
		   &photo->width, &photo->max_value);
	// Allocate memory for 2D pixel array
	photo->pixels = (int **)calloc(photo->width, sizeof(int *));

	if (!photo->pixels) {
		perror("Failed to allocate memory for pixel rows");
		free(photo);
		fclose(file);
		return;
	}

	for (int i = 0; i < photo->width; i++) {
		photo->pixels[i] = (int *)calloc(photo->length *
							photo->pixel_depth, sizeof(int));
		if (!photo->pixels[i]) {
			perror("Failed to allocate memory for pixel columns");
			for (int j = 0; j < i; j++) {
				free(photo->pixels[j]);
			}
			free(photo->pixels);
			free(photo);
			fclose(file);
			return;
		}
	}

	// Reads pixel data
	for (int i = 0; i < photo->width; i++) {
		for (int j = 0; j < photo->length * photo->pixel_depth; j++) {
			unsigned char pixel_value;
			fread(&pixel_value, sizeof(unsigned char), 1, file);
			photo->pixels[i][j] = pixel_value;
		}
	}
	fclose(file);
	printf("Loaded %s\n", file_name);
	// changes default selection
	photo->x1 = 0;
	photo->x2 = photo->length;
	photo->y1 = 0;
	photo->y2 = photo->width;
}

// decides which function needs to be called
// to handle the image loading
void handle_load(image_t *photo, char *file_name)
{
	char format[3] = {0};
	if (!file_name) {
		printf("Failed to load %s\n", file_name);
		return;
	}
	int l = strlen(file_name);
	file_name[l - 1] = '\0'; // fixes bad format
	// opens the file to check its format
	FILE *file = fopen(file_name, "rb");
	if (!file) {
		printf("Failed to load %s\n", file_name);
		free_image(photo);
		initialize_struct(photo);
		return; // Failed to load
	}
	fread(format, sizeof(char), 2, file);
	fclose(file);
	// calls right function based on format
	if (!strcmp(format, "P2")) {
		photo->pixel_depth = 1;
		text_file_image(photo, file_name);
	}
	if (!strcmp(format, "P3")) {
		photo->pixel_depth = 3;
		text_file_image(photo, file_name);
	}
	if (!strcmp(format, "P5")) {
		photo->pixel_depth = 1;
		binary_file_image(photo, file_name);
	}
	if (!strcmp(format, "P6")) {
		photo->pixel_depth = 3;
		binary_file_image(photo, file_name);
	}
}

// Called non-stop until EXIT stops the program
// Gets input, then calls the right function
void get_input(image_t *photo)
{
	char input[101] = {'\0'};
	char *mode;
	char *argument;
	static const char * const commands[] = {"APPLY", "LOAD", "CROP",
											"SAVE", "EXIT", "SELECT",
											"HISTOGRAM", "EQUALIZE", "ROTATE"};
	void (*handlers[])() = {handle_apply, handle_load, handle_crop, handle_save,
							handle_exit, handle_select, handle_histogram,
							handle_equalize, handle_rotate};
	int list_length = ARRAY_SIZE(commands);

	if (!fgets(input, sizeof(input), stdin)) {
		printf("EROARE LA CITIRE LINII");
		return;
	}
	// splits the input into 2 strings
	mode = strtok(input, " ");
	argument = strtok(NULL, "");
	if (!argument && strcmp(mode, "EXIT")) {
		mode[strlen(mode) - 1] = '\0';
	}
	// iterates through the whole list and matches the mode to the function
	for (int i = 0; i < list_length; i++) {
		if (!strcmp(mode, commands[i])) {
			handlers[i](photo, argument);
			break;
		}
		if (i == list_length - 1) {
			printf("Invalid command\n");
		}
	}
	// repeats itself until EXIT stops the execution
	get_input(photo);
}

int main(void)
{
	image_t photo;
	initialize_struct(&photo); // default values
	get_input(&photo); // starts loop
	free_image(&photo); // frees memory
	return 0;
}
