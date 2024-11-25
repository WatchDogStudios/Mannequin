set (NS_3RDPARTY_BC7ENC_RDO_SUPPORT ON CACHE BOOL "Whether to add support for the bc7 compression.")
mark_as_advanced(FORCE NS_3RDPARTY_BC7ENC_RDO_SUPPORT)

macro(ns_requires_bc7enc_rdo)
	
	ns_requires(NS_3RDPARTY_BC7ENC_RDO_SUPPORT)

endmacro()