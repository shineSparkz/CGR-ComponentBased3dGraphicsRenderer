#include "Image.h"

#include "gl_headers.h"
#include "LogFile.h"
#include "utils.h"

#include <sstream>

extern "C"
{
#include <jpeg/jpeglib.h>
#include <jpeg/jerror.h>
}
#include <cctype>

const std::string BAD_FILE = "BAD_FILE";

namespace jpg_info
{
	struct jpg_error_mgr
	{
		struct jpeg_error_mgr pub;
		jmp_buf setjmp_buffer;
	};

	typedef struct jpg_error_mgr * jpg_error_ptr;

	METHODDEF(void) jpeg_error_exit(j_common_ptr cinfo)
	{
		/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
		jpg_error_ptr err = (jpg_error_ptr)cinfo->err;

		// Always display the message.
		/* We could postpone this until after returning, if we chose. */
		(*cinfo->err->output_message) (cinfo);

		// Return control to the setjmp point
		longjmp(err->setjmp_buffer, 1);
	}
}

Image::Image() :
	m_Data(nullptr),
	m_Width(0),
	m_Height(0),
	m_NumBytes(0)
{
}

Image::Image(Image& rhs)
{
	this->m_Width = rhs.m_Width;
	this->m_Height = rhs.m_Height;
	this->m_NumBytes = rhs.m_NumBytes;

	// Deep copy
	size_t buff_size = (rhs.m_Width * rhs.m_Height) * rhs.m_NumBytes;
	m_Data = new byte[buff_size];

	for (size_t i = 0; i < buff_size; ++i)
	{
		m_Data[i] = rhs.m_Data[i];
	}
}

Image::~Image()
{
	SAFE_DELETE_ARRAY(m_Data);
}

bool Image::LoadImg(const char* file_name_no_ext)
{
	std::string fileIn = file_name_no_ext;

	std::string extension = util::get_file_extension(fileIn);
	if (extension == "")
	{
		WRITE_LOG("Error: There was no file extension: loading " + fileIn, "error");
		return false;
	}

	extension = util::str_to_lower(extension);

	if (extension == "tga")
	{
		FILE* file;
		byte type[4];
		byte info[6];

		file = fopen(file_name_no_ext, "rb");

		if (!file)
		{
			WRITE_LOG("Error: Cannot find TGA file : " + fileIn, "error");
			return false;
		}

		// Get first 3 values from TGA header and read them into type
		fread(&type, sizeof(byte), 3, file);
		
		// Skip 12 bytes into the header, we already have the first 3, and we skip the two word values
		fseek(file, 12, SEEK_SET);
		
		// Read the rest of the info into these arrays
		fread(&info, sizeof(byte), 6, file);

		// Find out the colour type ... 2nd part checks that it is not true colour and not monochrome data
		if (type[1] != 0 || (type[2] != 2 && type[2] != 3))
		{
			fclose(file);
			WRITE_LOG("Error: Incorrect type for TGA file : " + fileIn, "error");
			return false;
		}

		// Get the image widtth ( note the type from the tga header is a word, so use both pointers
		m_Width = info[0] + info[1] * 256;	// Little endian
		m_Height = info[2] + info[3] * 256;
		m_NumBytes = info[4] / 8;

		// Check byte count
		if (m_NumBytes < 3 || m_NumBytes > 4)
		{
			fclose(file);
			WRITE_LOG("Error: Incorrect num bytes for TGA file: " + fileIn, "error");
		}

		// Finally allocate the memory now that we have all of the information loaded from file
		dword image_size = m_Width * m_Height * m_NumBytes;
		m_Data = new byte[image_size];

		// read in data
		fread(m_Data, sizeof(byte), image_size, file);

		fclose(file);

#ifdef _DEBUG
		std::stringstream file_props;
		file_props << "Info: Image file: " << file_name_no_ext << " loaded ok, Width: " << m_Width << ", Height: " << m_Height << ", Size: " << image_size << std::endl;
		WRITE_LOG(file_props.str(), "none");
#endif
		return true;
	}
	else if (extension == "jpg" || extension == "jpeg")
	{
		FILE* file = fopen(file_name_no_ext, "rb");  //open the file
		if (!file)
		{
			fprintf(stderr, "Error reading JPEG file %s!!!", file_name_no_ext);
			return false;
		}

		struct jpg_info::jpg_error_mgr  jerr;
		struct jpeg_decompress_struct info;
		info.err = jpeg_std_error(&jerr.pub);
		jpeg_create_decompress(&info);

		jpeg_stdio_src(&info, file);
		jpeg_read_header(&info, TRUE);   
		jpeg_start_decompress(&info); 

		m_Width = info.output_width;
		m_Height = info.output_height;
		m_NumBytes = info.num_components;

		int size = m_Width * m_Height * m_NumBytes;
		m_Data = new byte[size];

		//  Read data into this buffer
		byte* p1 = m_Data;
		byte** p2 = &p1;
		int numlines = 0;
		while (info.output_scanline < info.output_height)
		{
			numlines = jpeg_read_scanlines(&info, p2, 1);
			*p2 += numlines * m_NumBytes * info.output_width;
		}

		// Finish decompressing this 
		jpeg_finish_decompress(&info);   
		jpeg_destroy_decompress(&info);

		fclose(file);

		return true;
	}
	else
	{
		WRITE_LOG("Error: Unsupported Image filetype: " + extension, "error");
		return false;
	}
}