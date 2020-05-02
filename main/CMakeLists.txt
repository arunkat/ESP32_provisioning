set(COMPONENT_SRCS "device.c"
	server.c
	lightABCdevice.c
	main.c
)
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

component_compile_definitions(SET_TRUSTED_CERT_IN_SAMPLES)
