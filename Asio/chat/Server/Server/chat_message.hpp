#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>

class ChatMessage
{
public:
	enum { header_length = 4 };
	enum { max_body_length = 512 };

	ChatMessage() :m_body_length(0) {}

	const char* data() const
	{
		return m_data;
	}

	char* data()
	{
		return m_data;
	}

	std::size_t length() const
	{
		return header_length + m_body_length;
	}

	const char* body() const
	{
		return m_data + header_length;
	}

	char* body()
	{
		return m_data + header_length;
	}

	std::size_t body_length() const
	{
		return m_body_length;
	}

	void body_length(std::size_t new_length)
	{
		m_body_length = new_length;
		if (m_body_length > max_body_length)
			m_body_length = max_body_length;
	}

	bool decode_header()
	{
		char header[header_length + 1] = "";
		std::strncat(header, m_data, header_length);
		m_body_length = std::atoi(header);
		if (m_body_length > max_body_length)
		{
			m_body_length = 0;
			return false;
		}
		return true;
	}

	void encode_header()
	{
		char header[header_length + 1] = "";
		std::sprintf(header, "%4d", static_cast<int>(m_body_length));
		std::memcpy(m_data, header, header_length);
	}

private:
	char m_data[header_length + max_body_length];
	std::size_t m_body_length;
};

#endif //CHAT_MESSAGE_HPP
