{
  'variables': {
	'version': 'v6.3.6.000'
  },
  'conditions':[
    ['OS=="linux"',{
		'variables': {
			'ctp_headers': '<(version)/linux64',
		},
		'targets': [{
			'target_name': 'copy',
			'copies': [{
				'destination': '<(PRODUCT_DIR)',
				'files': [
					'<(version)/linux64/thostmduserapi.so',
					'<(version)/linux64/thosttraderapi.so',
					'<(version)/linux64/error.xml',
					'<(version)/linux64/error.dtd',
				],
			}],
		}],
		'type': 'none',
	}],
    ['OS=="win"',{
		'variables': {
			'ctp_headers': '<(version)/linux64',
		},
		'targets': [{
			'target_name': 'copy',
			'copies': [{
				'destination': '<(PRODUCT_DIR)',
				'files': [
				    '<(version)/win64/thostmduserapi.dll',
					'<(version)/win64/thostmduserapi.lib',
					'<(version)/win64/thosttraderapi.dll',
					'<(version)/win64/thosttraderapi.lib',
					'<(version)/win64/error.xml',
					'<(version)/win64/error.dtd',
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
	  ],
	  "dependencies": [ "copy" ],
    }
  ]
}