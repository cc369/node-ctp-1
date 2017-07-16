{
  'variables': {
	'version': 'v6.3.6.000'
  },
  'conditions':[
    ['OS=="linux"',{
	  'variables': {
		'ctp_headers': '<(version)/linux',
		'market_library': 'thostmduserapi.lib',
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
	  ],
	  "libraries": [
		'<(PRODUCT_DIR)/<(market_library)',
	  ],
	  "dependencies": [ "copy" ],
    }
  ]
}