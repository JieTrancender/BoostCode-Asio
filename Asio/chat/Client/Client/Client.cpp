// Client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "chat_message.hpp"

using boost::asio::ip::tcp;

typedef std::deque<ChatMessage> chat_message_queue;

class ChatClient
{
public:
	ChatClient(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator) : m_io_service(io_service), m_socket(io_service)
	{
		do_connect(endpoint_iterator);
	}

	void write(const ChatMessage& msg)
	{
		m_io_service.post([this, msg]()
		{
			bool write_in_progress = !m_write_msgs.empty();
			m_write_msgs.push_back(msg);
			if (!write_in_progress)
			{
				do_write();
			}
		});
	}

	void close()
	{
		m_io_service.post([this]() { m_socket.close();  });
	}

private:
	void do_connect(tcp::resolver::iterator endpoint_iterator)
	{
		boost::asio::async_connect(m_socket, endpoint_iterator, [this](boost::system::error_code ec, tcp::resolver::iterator)
		{
			if (!ec)
			{
				do_read_header();
			}
		});
	}

	void do_read_header()
	{
		boost::asio::async_read(m_socket, boost::asio::buffer(m_read_msg.data(), ChatMessage::header_length), [this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec && m_read_msg.decode_header())
			{
				do_read_body();
			}
			else
			{
				m_socket.close();
			}
		});
	}

	void do_read_body()
	{
		boost::asio::async_read(m_socket, boost::asio::buffer(m_read_msg.body(), m_read_msg.body_length()), [this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::cout.write(m_read_msg.body(), m_read_msg.body_length());
				std::cout << "\n";
				do_read_header();
			}
			else
			{
				m_socket.close();
			}
		});
	}
	
	void do_write()
	{
		boost::asio::async_write(m_socket, boost::asio::buffer(m_write_msgs.front().data(), m_write_msgs.front().length()), [this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				m_write_msgs.pop_front();
				if (!m_write_msgs.empty())
				{
					do_write();
				}
			}
			else
			{
				m_socket.close();
			}
		});
	}

private:
	boost::asio::io_service& m_io_service;
	tcp::socket m_socket;
	ChatMessage m_read_msg;
	chat_message_queue m_write_msgs;
};

int main()
{
    return 0;
}

