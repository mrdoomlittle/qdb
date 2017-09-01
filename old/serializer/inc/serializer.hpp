
# ifndef __serializer__hpp
# define __serializer__hpp
# include <boost/cstdint.hpp>
# include <string.h>
namespace mdl {
struct serializer {
	serializer(char __typ) : typ(__typ) {}

	void operator&(std::size_t __size) {
		if (this-> typ == 's')
			this-> obj_size += __size;
		else this-> obj_size = __size;
	}

	template<typename __T>
	void operator<<(__T& __obj) {
		switch(this-> typ) {
			case 'r':
				memset(this-> serial + this-> curr_point, 0x0, this-> obj_size);
				for (std::size_t o = 0; o != this-> obj_size; o ++) {
					if (o == 0) this-> serial[this-> curr_point] = __obj & 0xFF;
					else
						this-> serial[this-> curr_point] = (__obj >> (o * 8)) & 0xFF;

					this-> curr_point ++;
				}
			break;
			case 'w':
				std::size_t begin_point = this-> curr_point;
				for (std::size_t o = 0; o != this-> obj_size; o ++) {
					__obj |= (__obj & 0xFF) | (this-> serial[this-> curr_point] << (o * 8));
					this-> curr_point ++;
				}
				memset(this-> serial + begin_point, 0x0, this-> obj_size);
			break;
		}
	}

	void reset() {
		this-> curr_point = 0;
		this-> obj_size = 0;
	}

	void operator|(char __typ) {
		this-> obj_size = 0;
		this-> curr_point = 0;
		this-> typ = __typ;
	}

	void init(std::size_t __bytes) {
		if (this-> serial != nullptr) {
			std::free(this-> serial);
			this-> serial = nullptr;
		}

		this-> serial = static_cast<boost::uint8_t *>(malloc(__bytes * sizeof(boost::uint8_t)));
		memset(this-> serial, 0x0, __bytes * sizeof(boost::uint8_t));
	}

	template <typename __T>
	std::size_t get_size(__T *__struct_ptr) {
		this-> typ = 's';
		__struct_ptr-> achieve(*this);

		std::size_t size = this-> obj_size;
		this-> obj_size = 0;
		return size;
	}

	boost::uint8_t* get_serial() {
		return this-> serial;
	}

	private:
	char typ = '\0';
	std::size_t obj_size = 0;
	std::size_t curr_point = 0;

	public:
	boost::uint8_t *serial = nullptr;
} ;
}

# endif /*__serializer__hpp*/

