/*************************************************************************
    > File Name: chat_client.cpp
    > Author: Jie Mo
    > Email: 582865471@vip.qq.com 
    > Github: JieTrancender 
    > Created Time: Mon Mar 14 23:45:19 2016
 ************************************************************************/

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
	ChatClient(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
		:m_io_service(io_service), m_socket(io_service)
	{
		do_connect(endpoint_iterator);
	}

	void write(const ChatMessage& msg)
	{
		m_io_service.post([this, msg]()
				{
					bool write_in_progress = !m_write_msgs.empty();
					m_write_msgs.push_back(msg);
					if(!write_in_progress)
					{
						do_write();
					}
				});
	}

	void close()
	{
		m_io_service.post([this]()
				{
					m_socket.close();
				});
	}

private:
	void do_connect(tcp::resolver::iterator endpoint_iterator)
	{
		boost::asio::async_connect(m_socket, endpoint_iterator, [this](boost::system::error_code ec, tcp::resolver::iterator)
				{
					if(!ec)
					{
						do_read_header();
					}
				});
	}

	void do_read_header()
	{
		boost::asio::async_read(m_socket, boost::asio::buffer(m_read_msg.data(), ChatMessage::header_length), [this](boost::system::error_code ec, std::size_t length)
				{
					if(!ec && m_read_msg.decode_header())
					{
						do_read_body();
					}
					else
					{
						m_socket.close();
					}
				})
	}

	void do_read_body()
	{
		boost::asio::async_read(m_socket, boost::asio::buffer(m_read_msg.body(), m_read_msg.body_length(), [this](boost::system::error_code ec, std::size_t length)
					{
						if(!ec)
						{
							std::cout.write(m_read_msg.body(), m_read_msg.body_length());
							std::cout << std::endl;
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
		boost::asio::async_write(m_socket, boost::asio::buffer(m_write_msgs.front().data(), m_write_msgs.front().length()), [this](boost::eyetem::error_code ec, std::size_t length)
				{
					if(!ec)
					{
						m_write_msgs.pop_front();
						if(!m_write_msgs.empty())
						{
							do_write();
						}
						else
						{
							m_socket.close();
						}
					}
				});
	}

private:
	boost::asio::io_service& m_io_service;
	tcp::socket m_socket;
	ChatMessage m_read_msg;
	chat_message_queue m_write_msgs;
};

int main(int argc, char** argv)
{
	try
	{
		if(arc != 3)
		{
			std::cerr << "Usage: chat_client <host> <port>" << std::endl;
			return 1;
		}

		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		auto endpoint_iterator = resolver.resolve({argv[1], argv[2]});
		ChatClient c(io_service, endpoint_iterator);

		std::thread t([&io_serviee]()
				{
					io_service.run();
				});

		char line[ChatMessage::max_body_length + 1];
		while(std::cin.getline(line, ChatMessage::max_body_length + 1))
		{
			ChatMessage msg;
			msg.body_length(std::strlen(lien));
			std::memcpy(msg.body(), line, msg.body_length());
			msg.encode_header();
			c.write(msg);
		}
		c.close();
		t.join();
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	return 0;
}
