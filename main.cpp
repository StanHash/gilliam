
#include <iostream>
#include <vector>
#include <iterator>
#include <fstream>

using byte_type = std::uint8_t;

template<unsigned Alignment, typename IntType>
constexpr IntType aligned(IntType value)
{
	return (value + (Alignment-1)) &~ (Alignment-1);
}

template<
	unsigned SlidingWindow = 4096,
	unsigned ReadAhead = 18
>
struct Lz
{
	static std::vector<byte_type> compress(const std::vector<byte_type>& input)
	{
		// https://github.com/pret/pokeruby/blob/master/tools/gbagfx/lz.c

		static const unsigned MIN_DIST = 2;

		if (input.empty())
			throw std::logic_error("compress input is empty!");

		const unsigned worstCaseDestSize = aligned<4>(4 + input.size() + ((input.size() + 7) / 8));

		std::vector<byte_type> output;
		output.reserve(worstCaseDestSize);

		// header
		output.push_back(0x10); // LZ compression type
		output.push_back(0xFF & (input.size()));
		output.push_back(0xFF & (input.size() >> 8));
		output.push_back(0xFF & (input.size() >> 16));

		auto it = input.begin();

		while (it != input.end())
		{
			auto flags = (output.push_back(0), std::prev(output.end()));

			for (unsigned i = 0; i < 8; ++i)
			{
				if (it == input.end())
					break;

				unsigned findDist = 0;
				unsigned findSize = 0;
				unsigned searchDist = MIN_DIST;

				while (searchDist <= std::distance(input.begin(), it) && searchDist <= SlidingWindow)
				{
					auto search = std::prev(it, searchDist);
					unsigned searchSize = 0;

					while ((searchSize < ReadAhead)
						&& (std::next(it, searchSize) != input.end())
						&& (*std::next(search, searchSize) == *std::next(it, searchSize)))
					{
						searchSize++;
					}

					if (searchSize > findSize)
					{
						findDist = searchDist;
						findSize = searchSize;

						if (searchSize == 18)
							break;
					}

					searchDist++;
				}

				if (findSize >= 3)
				{
					*flags |= (0x80 >> i);

					it = std::next(it, findSize);

					output.push_back(
						(((findSize-3) & 0xF) << 4) |
						(((findDist-1) >> 8) & 0xF));

					output.push_back(findDist-1);
				}
				else
				{
					output.push_back(*it++);
				}
			}
		}

		// Pad to multiple of 4 bytes.
		if (unsigned remainder = output.size() % 4)
		{
			for (unsigned i = 0; i < 4 - remainder; i++)
				output.push_back(0);
		}

		return output;
	}
};

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "usage:" << std::endl;
		std::cerr << "  " << argv[0] << " <file path>" << std::endl;
		return 1;
	}

	try
	{
		std::ifstream fin(argv[1], std::ios::binary | std::ios::in);

		if (!fin.is_open())
		{
			std::cerr << "couldn't open file:" << std::endl;
			std::cerr << "  " << argv[1] << std::endl;

			return 2;
		}

		fin.seekg(0, std::ios::end);
		std::size_t sz = fin.tellg();

		fin.seekg(0, std::ios::beg);

		std::vector<byte_type> in(sz, 0);

		if (!fin.read(reinterpret_cast<char*>(in.data()), sz))
		{
			std::cerr << "couldn't read from file:" << std::endl;
			std::cerr << "  " << argv[1] << std::endl;

			return 3;
		}

		fin.close();

		std::vector<byte_type> out = Lz<>::compress(in);
		std::cout.write(reinterpret_cast<const char*>(out.data()), out.size());
	}
	catch (const std::exception& e)
	{
		std::cerr << "an error occured:" << std::endl;
		std::cerr << "  " << e.what() << std::endl;

		return 4;
	}

	return 0;
}
