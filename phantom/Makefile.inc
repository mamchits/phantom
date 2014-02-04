include phantom/module.mk
others_pd += phantom/module.mk

# ===
# MODULE,dir,package_suffix,local_libs,nonlocal_libs

$(eval $(call MODULE,io_client))
$(eval $(call MODULE,io_client/ipv4))
$(eval $(call MODULE,io_client/ipv6))
$(eval $(call MODULE,io_client/local))
$(eval $(call MODULE,io_client/proto_none))
$(eval $(call MODULE,io_client/proto_fcgi))

$(eval $(call MODULE,io_monitor))

$(eval $(call MODULE,io_stream))
$(eval $(call MODULE,io_stream/ipv4))
$(eval $(call MODULE,io_stream/ipv6))
#$(eval $(call MODULE,io_stream/local))
$(eval $(call MODULE,io_stream/proto_monitor))
$(eval $(call MODULE,io_stream/proto_echo))
$(eval $(call MODULE,io_stream/proto_http))
$(eval $(call MODULE,io_stream/proto_http/handler_null))
$(eval $(call MODULE,io_stream/proto_http/handler_static))
$(eval $(call MODULE,io_stream/proto_http/handler_proxy))
$(eval $(call MODULE,io_stream/proto_http/handler_fcgi))
$(eval $(call MODULE,io_stream/proto_http/handler_monitor))

$(eval $(call MODULE,io_benchmark,,,m))
$(eval $(call MODULE,io_benchmark/method_stream))
$(eval $(call MODULE,io_benchmark/method_stream/ipv4))
$(eval $(call MODULE,io_benchmark/method_stream/ipv6))
$(eval $(call MODULE,io_benchmark/method_stream/local))
$(eval $(call MODULE,io_benchmark/method_stream/proto_none))
$(eval $(call MODULE,io_benchmark/method_stream/proto_http))
$(eval $(call MODULE,io_benchmark/method_stream/source_random))
$(eval $(call MODULE,io_benchmark/method_stream/source_log))

$(eval $(call MODULE,ssl,_ssl,ssl,ssl crypto pthread))
$(eval $(call MODULE,io_stream/transport_ssl,_ssl))
$(eval $(call MODULE,io_benchmark/method_stream/transport_ssl,_ssl))

$(eval $(call MODULE,debug,_debug,debug,bfd))

$(targets_phantom_module): phantom/module.mk

# ===

srcs_phantom = $(shell find phantom/ -maxdepth 1 -name "*.C" | sort) \
	phantom/jemalloc/jemalloc_.C

objs_phantom = $(srcs_phantom:%.C=%.o)

targets_phantom = bin/phantom
tmps_phantom = $(objs_phantom)

libs_phantom = lib/libpd-pi.a lib/libpd-http.a lib/libpd-bq.a lib/libpd-base.a

CFLAGS.phantom/jemalloc/jemalloc.c = -Wno-unused
CXXFLAGS.phantom/jemalloc/jemalloc_.C = -Wno-unused -Wno-unused-parameter

bin/phantom: $(objs_phantom) $(libs_phantom)
	$(CXX) -rdynamic $(objs_phantom) \
		-Wl,--whole-archive $(libs_phantom) -Wl,--no-whole-archive \
		-lpthread -ldl -o $(@)

#$(targets_phantom) $(tmps_phantom): phantom/Makefile.inc
targets += $(targets_phantom)
tmps += $(tmps_phantom)

utilities += bin/phantom

dirs_pd += phantom
