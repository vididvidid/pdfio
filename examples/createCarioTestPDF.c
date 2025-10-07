#include <stdio.h>
#include "pdfio.h"

//
// 'main()' - Create a PDF with nested, independently-styled boxes.
//
int main(int argc, char *argv[])
{
  // 1. Check for command-line argument
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <output-path.pdf>\n", argv[0]);
    return (1);
  }
  const char *output_filename = argv[1];

  // Page and box dimensions
  pdfio_rect_t media_box = {0.0, 0.0, 612.0, 792.0};

  // Outer filled box
  const double outer_x = 100.0;
  const double outer_y = 100.0;
  const double outer_w = 412.0;
  const double outer_h = 692.0;
  const double outer_r = 0.8; // Gray
  const double outer_g = 0.8;
  const double outer_b = 0.8;

  // Inner stroked box
  const double inner_x = 150.0;
  const double inner_y = 150.0;
  const double inner_w = 312.0;
  const double inner_h = 592.0;
  const double inner_r = 1.0; // Red
  const double inner_g = 0.0;
  const double inner_b = 0.0;
  const double line_width = 4.0;

  // 2. Create the PDF file
  pdfio_file_t *pdf = pdfioFileCreate(output_filename, NULL, &media_box, NULL, NULL, NULL);
  if (!pdf)
  {
    fprintf(stderr, "Error: Could not create PDF file '%s'.\n", output_filename);
    return (1);
  }

  // 3. Create a page
  pdfio_stream_t *page_stream = pdfioFileCreatePage(pdf, NULL);
  if (!page_stream)
  {
    fprintf(stderr, "Error: Could not create page in '%s'.\n", output_filename);
    pdfioFileClose(pdf);
    return (1);
  }

 // 4. Write PDF commands to draw a filled and stroked rectangle
  pdfioStreamPrintf(page_stream, "q\n");                // Save graphics state

  pdfioStreamPrintf(page_stream, "8 w\n");                // Set a thick line width
  pdfioStreamPrintf(page_stream, "0 0 0.8 RG\n");         // Set stroke color to dark blue
  pdfioStreamPrintf(page_stream, "0.7 0.9 1 rg\n");       // Set fill color to light blue

  // Define a rectangle
  pdfioStreamPrintf(page_stream, "150 400 312 200 re\n");

  // Fill and stroke the path in one command
  pdfioStreamPrintf(page_stream, "B\n");

  pdfioStreamPrintf(page_stream, "Q\n");                // Restore graphics state


  // 5. Close and save the file
  pdfioStreamClose(page_stream);
  pdfioFileClose(pdf);

  printf("Successfully created %s\n", output_filename);

  return (0);
}
