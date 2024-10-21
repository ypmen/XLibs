/**
 * @author Yunpeng Men
 * @email ypmen@mpifr-bonn.mpg.de
 * @create date 2024-04-03 19:45:10
 * @modify date 2024-04-03 19:45:10
 * @desc [description]
 */

#ifndef DADA_H
#define DADA_H

#include "dada_def.h"
#include "dada_hdu.h"
#include "json.hpp"

namespace PSRDADA {
	bool hdu_connect(dada_hdu_t *hdu, multilog_t *log, int connection_attempts=1000)
	{
		int connected = dada_hdu_connect(hdu);
		while (connected < 0)
		{
			connection_attempts--;
			if (connection_attempts < 0)
			{
				multilog(log, LOG_ERR, "Could not connect to dada\n");
				return false;
			}

			multilog(log, LOG_WARNING, "Failed to connect to HDU %d attempts remaining\n", connection_attempts);

			sleep(1);

			connected = dada_hdu_connect(hdu);
		}

		return true;
	}

	bool read_header(dada_hdu_t *hdu, multilog_t *log, nlohmann::json &h)
	{
		uint64_t header_size = 0;
		char* dada_header = ipcbuf_get_next_read(hdu->header_block, &header_size);
		if (!dada_header)
		{
			multilog(log, LOG_ERR, "Could not get next header\n");
			return false;
		}

		h = nlohmann::json::parse(dada_header);

		if (ipcbuf_mark_cleared(hdu->header_block) < 0)
		{
			multilog(log, LOG_ERR, "Could not mark cleared header block\n");
			return false;
		}

		return true;
	}

	bool write_header(dada_hdu_t *hdu, multilog_t *log, nlohmann::json &h)
	{
		std::string s = h.dump();

		uint64_t header_size = ipcbuf_get_bufsz (hdu->header_block);

		char* header = NULL;
		if (header_size)
		{
			header = ipcbuf_get_next_write (hdu->header_block);
			if (!header)
			{
				multilog(log, LOG_ERR, "Could not get next header block\n");
				return false;
			}

			std::memcpy(header, s.c_str(), s.size());

			if (ipcbuf_mark_filled (hdu->header_block, header_size) < 0)
			{
				multilog(log, LOG_ERR, "Could not mark filled header block\n");
				return false;
			}
		}
		else
		{
			multilog(log, LOG_WARNING, "Header size is zero\n");
			return false;
		}

		return true;
	}

	class lock_guard
	{
	public:
		lock_guard(dada_hdu_t *hdu, int flag=0)
		{
			_log = multilog_open("dada_lock_guard", false);

			_hdu = hdu;
			_flag = flag;

			if (_flag == 1)
			{
				if (dada_hdu_lock_write(_hdu) < 0)
				{
					multilog(_log, LOG_ERR, "Could not lock write dada\n");
				}
			}
			else
			{
				if (dada_hdu_lock_read(_hdu) < 0)
				{
					multilog(_log, LOG_ERR, "Could not lock read dada\n");
				}
			}
		}

		~lock_guard()
		{
			if (_flag == 1)
			{
				if (dada_hdu_unlock_write(_hdu) < 0)
				{
					multilog(_log, LOG_ERR, "Could not unlock write dada\n");
				}
			}
			else
			{
				if (dada_hdu_unlock_read(_hdu) < 0)
				{
					multilog(_log, LOG_ERR, "Could not unlock read dada\n");
				}
			}

			multilog_close(_log);
		}

	private:
		multilog_t* _log;
		dada_hdu_t *_hdu;
		int _flag;
	};

	/* psrdada reader */
	class Reader
	{
	public:
		Reader(const std::string &key)
		{
			sscanf (key.c_str(), "%x", &dada_key);

			log = multilog_open("dada_reader", false);
			hdu = dada_hdu_create(log);
			dada_hdu_set_key(hdu, dada_key);
			PSRDADA::hdu_connect(hdu, log);
		}

		~Reader()
		{
			dada_hdu_disconnect(hdu);
			dada_hdu_destroy(hdu);
			multilog_close(log);
		}

		bool prepare(nlohmann::json &header)
		{
			PSRDADA::lock_guard lock(hdu, 0);

			if (!PSRDADA::read_header(hdu, log, header)) return false;

			return true;
		}

		size_t run(char *data, size_t n)
		{
			PSRDADA::lock_guard lock(hdu, 0);

			return ipcio_read(hdu->data_block, data, n);
		}

		size_t get_bufsz()
		{
			return ipcbuf_get_bufsz((ipcbuf_t *)(hdu->data_block));
		}

	private:
		key_t dada_key;
		multilog_t* log;
		dada_hdu_t* hdu;
	};

	/* psrdada writer */
	class Writer
	{
	public:
		Writer(){};
		
		Writer(const std::string &key)
		{
			sscanf (key.c_str(), "%x", &dada_key);

			log = multilog_open("dada_writer", false);
			hdu = dada_hdu_create(log);
			dada_hdu_set_key(hdu, dada_key);
			PSRDADA::hdu_connect(hdu, log);
		}

		~Writer()
		{
			dada_hdu_disconnect(hdu);
			dada_hdu_destroy(hdu);
			multilog_close(log);
		}

		void setup(const std::string &key)
		{
			sscanf (key.c_str(), "%x", &dada_key);

			log = multilog_open("dada_writer", false);
			hdu = dada_hdu_create(log);
			dada_hdu_set_key(hdu, dada_key);
			PSRDADA::hdu_connect(hdu, log);
		}

		bool prepare(nlohmann::json &header)
		{
			PSRDADA::lock_guard lock(hdu, 1);

			if (!PSRDADA::write_header(hdu, log, header)) return false;

			return true;
		}

		size_t run(char *data, size_t n)
		{
			PSRDADA::lock_guard lock(hdu, 1);

			return ipcio_write(hdu->data_block, data, n);
		}

		size_t get_bufsz()
		{
			return ipcbuf_get_bufsz((ipcbuf_t *)(hdu->data_block));
		}

	private:
		key_t dada_key;
		multilog_t* log;
		dada_hdu_t* hdu;
	};
}

#endif /* DADA_H */
