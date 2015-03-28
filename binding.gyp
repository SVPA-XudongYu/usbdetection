{
  "targets": [
    {
      "target_name": "detection",
      "sources": [
        "src/detection.cpp",
        "src/detection.h",
        "src/deviceList.cpp"
      ],
      'conditions': [
        ['OS=="win"',
          {
            'sources': [
              "src/detection_win.cpp"
            ],
            'include_dirs+': 
            [
              # Not needed now
            ]
          }
        ],
        ['OS=="mac"',
          {
            'sources': [
              "src/detection_mac.cpp"
            ]
          }
        ],
        ['OS=="linux"',
          {
            'sources': [
              "src/detection_linux.cpp"
            ],
            'link_settings': {
              'libraries': [
                '-ludev'
              ]
            }
          }
        ]
      ]
    }
  ]
}
