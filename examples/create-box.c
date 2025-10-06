
#include "pdfio.h"
#include <stdio.h>
#include <string.h>

int
main(int  argc,
     char *argv[])
{
  pdfio_file_t	*pdf;		// PDF file
  pdfio_dict_t	*page_dict;	// Page dictionary
  pdfio_stream_t *st;		// Page content stream
  pdfio_rect_t	mediabox = { 0.0, 0.0, 612.0, 792.0 }; // US Letter

  // Check command-line...
  if (argc != 2)
  {
    puts("Usage: ./create-box output.pdf");
    return (1);
  }

  // Create the PDF file...
  // This writes the initial "%PDF-1.7" header.
  if ((pdf = pdfioFileCreate(argv[1], "1.7", &mediabox, NULL, NULL, NULL)) == NULL)
    return (1);

  // Create a page dictionary...
  // pdfio will automatically add the /Type, /Parent, and /MediaBox keys.
  page_dict = pdfioDictCreate(pdf);

  // Create the page, which also creates the content stream object for us.
  // This returns a stream handle we can write drawing commands to.
  if ((st = pdfioFileCreatePage(pdf, page_dict)) == NULL)
  {
    pdfioFileClose(pdf);
    return (1);
  }

  // Write the drawing commands to the stream...
  // 1 0 0 rg = Set fill color to Red
  pdfioStreamPrintf(st, "1 0 0 rg\n");
  // 100 100 200 150 re = Define a rectangle
  pdfioStreamPrintf(st, "100 100 200 150 re\n");
  // f = Fill the current path (the rectangle)
  pdfioStreamPrintf(st, "f\n");

  // Close the stream - this is a required step!
  pdfioStreamClose(st);

  // Close the PDF file. This is where PDFio writes all the objects,
  // the cross-reference table (xref), and the trailer.
  if (pdfioFileClose(pdf))
  {
    printf("Successfully created '%s'.\n", argv[1]);
    return (0);
  }
  else
  {
    fprintf(stderr, "Error creating '%s'.\n", argv[1]);
    return (1);
  }
}
