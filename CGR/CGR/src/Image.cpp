#include "Image.h"

#include "gl_headers.h"
#include "LogFile.h"
#include "utils.h"

#include <sstream>

const std::string BAD_FILE = "BAD_FILE";

Image::Image()
{
}

Image::Image(Image& rhs)
{
	this->width = rhs.width;
	this->height = rhs.height;
	this->num_bytes = rhs.num_bytes;

	// Deep copy
	size_t buff_size = (rhs.width * rhs.height) * rhs.num_bytes;
	this->data = new byte[buff_size];

	for (size_t i = 0; i < buff_size; ++i)
	{
		this->data[i] = rhs.data[i];
	}
}

Image::~Image()
{
	SAFE_DELETE_ARRAY(data);
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
		width = info[0] + info[1] * 256;	// Little endian
		height = info[2] + info[3] * 256;
		this->num_bytes = info[4] / 8;

		// Check byte count
		if (num_bytes < 3 || num_bytes > 4)
		{
			fclose(file);
			WRITE_LOG("Error: Incorrect num bytes for TGA file: " + fileIn, "error");
		}

		// Finally allocate the memory now that we have all of the information loaded from file
		dword image_size = width * height * num_bytes;
		data = new byte[image_size];

		// read in data
		fread(data, sizeof(byte), image_size, file);

		fclose(file);

#ifdef _DEBUG
		std::stringstream file_props;
		file_props << "Info: Image file: " << file_name_no_ext << " loaded ok, Width: " << width << ", Height: " << height << ", Size: " << image_size << std::endl;
		WRITE_LOG(file_props.str(), "none");
#endif
		return true;
	}
	//else if ... todo
	else
	{
		WRITE_LOG("Error: Unsupported Image filetype: " + extension, "error");
		return false;
	}
}