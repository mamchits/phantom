include phantom/module.mk
others_pd += phantom/module.mk

# ===
# MODULE,dir,package_suffix,local_libs,nonlocal_libs

$(eval $(call MODULE,io_client))
$(eval $(call MODULE,io_client/ipv4))
$(eval $(call MODULE,io_client/ipv6))
$(eval $(call MODULE,io_client/local))
$(eval $(call MODULE,io_client/proto_none,,,-lpthread))
$(eval $(call MODULE,io_client/proto_fcgi,,,-lpthread))

$(eval $(call MODULE,io_stream,,,-lpthread))
$(eval $(call MODULE,io_stream/ipv4))
$(eval $(call MODULE,io_stream/ipv6))
#$(eval $(call MODULE,io_stream/local))
$(eval $(call MODULE,io_stream/proto_monitor))
$(eval $(call MODULE,io_stream/proto_echo))
$(eval $(call MODULE,io_stream/proto_http,,,-lpthread))
$(eval $(call MODULE,io_stream/proto_http/handler_null))
$(eval $(call MODULE,io_stream/proto_http/handler_static))
$(eval $(call MODULE,io_stream/proto_http/handler_proxy,,,-lpthread))
$(eval $(call MODULE,io_stream/proto_http/handler_fcgi))

$(eval $(call MODULE,io_benchmark,,,-lm -lpthread))
$(eval $(call MODULE,io_benchmark/method_stream,,,-lpthread))
$(eval $(call MODULE,io_benchmark/method_stream/ipv4,,,-lpthread))
$(eval $(call MODULE,io_benchmark/method_stream/ipv6,,,-lpthread))
$(eval $(call MODULE,io_benchmark/method_stream/local))
$(eval $(call MODULE,io_benchmark/method_stream/proto_none))
$(eval $(call MODULE,io_benchmark/method_stream/proto_http,,,-lpthread))
$(eval $(call MODULE,io_benchmark/method_stream/source_random,,,-lpthread))
$(eval $(call MODULE,io_benchmark/method_stream/source_log))

$(eval $(call MODULE,ssl,_ssl,ssl,-lssl -lcrypto -lpthread))
$(eval $(call MODULE,io_stream/transport_ssl,_ssl))
$(eval $(call MODULE,io_benchmark/method_stream/transport_ssl,_ssl))

$(eval $(call MODULE,debug,_debug,debug,-lbfd))

$(targets_phantom_module): phantom/module.mk

# ===

phantom = \
	phantom obj setup_module setup_caps setup_daemon module \
	scheduler scheduler_simple scheduler_combined io \
	io_logger_file logger_stderr logger_file arena jemalloc/jemalloc

objs_phantom = $(phantom:%=phantom/%.o)
targets_phantom = bin/phantom
tmps_phantom = $(objs_phantom)

libs_phantom = lib/libpd-http.a lib/libpd-bq.a lib/libpd-base.a

CFLAGS.phantom/jemalloc/jemalloc.c = -Wno-unused

bin/phantom: $(objs_phantom) $(objs_jemalloc) $(libs_phantom)
	$(CXX) -rdynamic $(objs_phantom) $(objs_jemalloc) \
		-Wl,--whole-archive $(libs_phantom) -Wl,--no-whole-archive \
		-lpthread -ldl -o $(@)

#$(targets_phantom) $(tmps_phantom): phantom/Makefile.inc
targets += $(targets_phantom)
tmps += $(tmps_phantom)

utilities += bin/phantom

dirs_pd += phantom
