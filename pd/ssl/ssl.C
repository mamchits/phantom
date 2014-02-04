// This file is part of the pd::ssl library.
// Copyright (C) 2011-2014, Eugene Mamchits <mamchits@yandex-team.ru>.
// Copyright (C) 2011-2014, YANDEX LLC.
// This library may be distributed under the terms of the GNU LGPL 2.1.
// See the file ‘COPYING’ or ‘http://www.gnu.org/licenses/lgpl-2.1.html’.

#include "ssl.H"

#include "log.I"

#include <pd/bq/bq_cont.H>

#include <pd/base/exception.H>
#include <pd/base/string_file.H>

#include <pthread.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/engine.h>

namespace pd {

namespace {

struct mgr_t {
	static unsigned types;
	static pthread_rwlock_t *locks;

	static void lock_func(int mode, int type, const char *, int) {

		if((unsigned)type > types)
			return;

		pthread_rwlock_t *lock = &locks[type];

		if(mode & CRYPTO_LOCK) {
			if(mode & CRYPTO_READ)
				pthread_rwlock_rdlock(lock);
			else
				pthread_rwlock_wrlock(lock);
		}
		else
			pthread_rwlock_unlock(lock);
	}

	static unsigned long id_func(void) {
		return (unsigned long)bq_cont_current ?: (unsigned long)pthread_self();
	}

	inline mgr_t() throw() {
		SSL_library_init();
		SSL_load_error_strings();
		ENGINE_load_builtin_engines();
		OpenSSL_add_all_algorithms();

		types = CRYPTO_num_locks();
		locks = (pthread_rwlock_t *)malloc(types * sizeof(pthread_rwlock_t));

		for(unsigned i = 0; i < types; ++i)
			pthread_rwlock_init(&locks[i], NULL);

		CRYPTO_set_locking_callback(&lock_func);
		CRYPTO_set_id_callback(&id_func);
	}

	inline ~mgr_t() throw() {
		CRYPTO_set_id_callback(NULL);
		CRYPTO_set_locking_callback(NULL);

		for(unsigned i = 0; i < types; ++i)
			pthread_rwlock_destroy(&locks[i]);

		free(locks);

		locks = NULL;
		types = 0;
	}
};

unsigned mgr_t::types = 0;
pthread_rwlock_t *mgr_t::locks = NULL;
static mgr_t const mgr;

class bio_t {
	BIO *val;

public:
	inline bio_t(string_t const &buf) :
		val(BIO_new_mem_buf((void *)buf.ptr(), buf.size())) {

		if(!val) {
			log_openssl_error(log::error);
			throw exception_log_t(log::error, "BIO_new_mem_buf");
		}
	}

	inline ~bio_t() throw() {
		BIO_free(val);
	}

	inline operator BIO *() { return val; }
};

} // namespace

ssl_auth_t::ssl_auth_t(
	string_t const &key_fname, string_t const &cert_fname
) : key(), cert() {

	if(key_fname)
		key = string_file(key_fname.str());

	if(cert_fname)
		cert = string_file(cert_fname.str());
}

ssl_auth_t::~ssl_auth_t() throw() { }

ssl_ctx_t::ssl_ctx_t(
	mode_t _mode, ssl_auth_t const *auth, string_t const &ciphers
) : internal(NULL) {
	SSL_CTX *ctx = SSL_CTX_new(
		_mode == client ? SSLv23_client_method() : SSLv23_server_method()
	);

	if(!ctx) {
		log_openssl_error(log::error);
		throw exception_log_t(log::error, "SSL_CTX_new");
	}

	if(auth) {
		try {
			if(auth->cert) {
				bio_t cert_bio(auth->cert);

				X509 *x = PEM_read_bio_X509_AUX(cert_bio, NULL, NULL, NULL);

				if(!x) {
					log_openssl_error(log::error);
					throw exception_log_t(log::error, "cert, PEM_read_bio_X509");
				}

				if(SSL_CTX_use_certificate(ctx, x) <= 0) {
					log_openssl_error(log::error);
					throw exception_log_t(log::error, "SSL_CTX_use_certificate");
				}

				X509_free(x);

				while(!BIO_eof(cert_bio)) {
					x = PEM_read_bio_X509(cert_bio, NULL, NULL, NULL);

					if(!x) {
						log_openssl_error(log::error);
						throw exception_log_t(log::error, "cert_chain, PEM_read_bio_X509");
					}

					if(SSL_CTX_add_extra_chain_cert(ctx, x) <= 0) {
						X509_free(x);

						log_openssl_error(log::error);
						throw exception_log_t(log::error, "SSL_CTX_add_extra_chain_cert");
					}

					// X509_free(x); // ????
				}
			}

			if(auth->key) {
				bio_t key_bio(auth->key);

				EVP_PKEY *pkey = PEM_read_bio_PrivateKey(key_bio, NULL, NULL, NULL);
				if(!pkey) {
					log_openssl_error(log::error);
					throw exception_log_t(log::error, "PEM_read_bio_PrivateKey");
				}

				if(SSL_CTX_use_PrivateKey(ctx, pkey) <= 0) {
					log_openssl_error(log::error);
					throw exception_log_t(log::error, "SSL_CTX_use_PrivateKey");
				}

				EVP_PKEY_free(pkey);
			}


			if(_mode == server) {
				SSL_CTX_set_options(ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
				SSL_CTX_set_options(ctx, SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);
				SSL_CTX_set_options(ctx, SSL_OP_TLS_ROLLBACK_BUG);
				SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

				if(ciphers) {
					MKCSTR(_ciphers, ciphers);

					if(SSL_CTX_set_cipher_list(ctx, _ciphers) <= 0) {
						log_openssl_error(log::error);
						throw exception_log_t(log::error, "SSL_CTX_set_cipher_list");
					}
				}
/*
				SSL_CTX_set_session_cache_mode(ctx, ...);
				SSL_CTX_set_timeout(ctx, ...);
*/
			}

/*
			SSL_CTX_set_default_verify_paths(ctx);
			SSL_CTX_load_verify_locations(ctx, NULL, "/etc/ssl/certs");
			SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
*/
		}
		catch(...) {
			SSL_CTX_free(ctx);
			throw;
		}
	}

	internal = ctx;
}

ssl_ctx_t::~ssl_ctx_t() throw() {
	SSL_CTX_free((SSL_CTX *)internal);
}

} // namespace pd
