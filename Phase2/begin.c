#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo.h>

void text_to_image(const char* input_file, const char* font, int font_size, const char* output_file) {
    // Open input file
    FILE *file = fopen(input_file, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open input file\n");
        return;
    }

    // Read text from input file
    char *text = NULL;
    size_t text_length = 0;
    ssize_t read;
    while ((read = getline(&text, &text_length, file)) != -1) {}

    // Close input file
    fclose(file);

    // Initialize Cairo surface
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 50);
    cairo_t *cr = cairo_create(surface);

    // Set font options
    cairo_select_font_face(cr, font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, font_size);

    // Draw text
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black color
    cairo_move_to(cr, 10, 40);
    cairo_show_text(cr, text);

    // Write to PNG file
    cairo_surface_write_to_png(surface, output_file);

    // Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    free(text);
}

int main() {
    text_to_image("input.txt", "Sans", 20, "output.png");
    return 0;
}
