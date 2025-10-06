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

  // 4. Write PDF commands to draw the nested boxes
  // Start with the outer box
  pdfioStreamPrintf(page_stream, "q\n");                                   // Save initial graphics state
  pdfioStreamPrintf(page_stream, "%.2f %.2f %.2f rg\n",
                    outer_r, outer_g, outer_b);                            // Set fill color to gray
  pdfioStreamPrintf(page_stream, "%.2f %.2f %.2f %.2f re\n",
                    outer_x, outer_y, outer_w, outer_h);                   // Define outer rectangle
  pdfioStreamPrintf(page_stream, "f\n");                                   // Fill it

  // Now, draw the inner box in a nested state
  pdfioStreamPrintf(page_stream, "q\n");                                   // Save current state (gray fill is active)
  pdfioStreamPrintf(page_stream, "%.2f w\n", line_width);                 // Set line width to 4 points
  pdfioStreamPrintf(page_stream, "%.2f %.2f %.2f RG\n",
                    inner_r, inner_g, inner_b);                            // Set STROKE color to red ('RG' is for stroke)
  pdfioStreamPrintf(page_stream, "%.2f %.2f %.2f %.2f re\n",
                    inner_x, inner_y, inner_w, inner_h);                   // Define inner rectangle
  pdfioStreamPrintf(page_stream, "S\n");                                   // Stroke it
  pdfioStreamPrintf(page_stream, "Q\n");                                   // Restore state (removes red stroke color and line width)

  pdfioStreamPrintf(page_stream, "Q\n");                                   // Restore initial state

  // 5. Close and save the file
  pdfioStreamClose(page_stream);
  pdfioFileClose(pdf);

  printf("Successfully created %s\n", output_filename);

  return (0);
}
