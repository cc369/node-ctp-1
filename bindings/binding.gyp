{
  'variables': {
	'version': 'v6.3.6.000'
  },
  'conditions':[
    ['OS=="linux"',{
	  'variables': {
		'ctp_headers': '<(version)/linux/x64',
		'market_library': 'thostmduserapi.so',
		'trader_library': 'thosttraderapi.so',
	  }, 
          'targets': [{
		'target_name': 'copy',
		'copies': [{
		  'destination': '<(PRODUCT_DIR)',
		  'files': [
			'<(version)/linux/x64/thostmduserapi.so',
			'<(version)/linux/x64/thosttraderapi.so',
			'<(version)/linux/x64/error.xml',
			'<(version)/linux/x64/error.dtd',
		  ],
		}],
	  }],
	  'type': 'none',
	}],
    ['OS=="win"',{
	  'variables': {
		'ctp_headers': '<(version)/win32/x64',
		'market_library': 'thostmduserapi.lib',
		'trader_library': 'thosttraderapi.lib',
      },
      'targets': [{
		'target_name': 'copy',
		'copies': [{
		  'destination': '<(PRODUCT_DIR)',
			'files': [
			  '<(version)/win32/x64/thostmduserapi.dll',
			  '<(version)/win32/x64/thostmduserapi.lib',
			  '<(version)/win32/x64/thosttraderapi.dll',
		      '<(version)/win32/x64/thosttraderapi.lib',
			  '<(version)/win32/x64/error.xml',
			  '<(version)/win32/x64/error.dtd',
			],
		}],
		'type': 'none',
	  }]
	}],
  ],
  "targets": [
    {
	  'include_dirs': [
		'<(ctp_headers)',
	  ],
      "target_name": "ctp",
      "sources": [ 
		"CtpPrice.cc", 
		"CtpPrice.h", 
		"module.cpp",
		"V8Helper.h",
		"GetObjectField.cpp",
		"readerwritequeue.h",
		"atomicops.h",
		"CtpModule.h",
		"CtpModule.cpp",
		"CtpTrade.h",
		"CtpTrade.cc",
	  ],
          'conditions': [
            [ 'OS=="win"', {
	      "libraries": [
		'<(PRODUCT_DIR)/<(market_library)',
		'<(PRODUCT_DIR)/<(trader_library)',
	      ],
            }],
            [ 'OS=="linux"', {
              "libraries": [
		'-l:<(market_library)',
		'-l:<(trader_library)',
	      ],
              "library_dirs": [
                '<(PRODUCT_DIR)',
              ],
              "ldflags": [
                "-Wl,-rpath '-Wl,$$ORIGIN'",
              ], 
            }],
          ],
	  "dependencies": [ "copy" ],
    }
  ]
}
