- 设计：Screen Time App on Windows

- 界面部分：Tabs

  - 分组：用户可以将使用的软件进行分组，以便更好地管理和控制自己的应用程序。
    - 分组
      - get_all_category
      - rename_category
      - add_category
      - delete_category
    - 应用
      - get_apps_under_category
      - move_app_to_category
  - 提醒：当用户使用某个应用分组的时间达到指定的使用时间时，程序会提醒用户自定义内容，以便用户可以更好地控制自己的应用程序使用时间。
    - get_all_reminder
    - add_reminder
    - remove_reminder
    - modify_reminder
  - 统计：用户可以选择指定的开始日期和结束日期，以显示在该时间段内使用软件的时长统计图，以便用户更好地了解自己的应用程序使用情况。
    - get_usage_app_timespan
  - 时间轴：时间轴可以展示用户在指定日期内使用进程的时间轴，以便用户更好地了解自己的应用程序使用情况。
    - get_usage_app_timeline_day

- 后台进程部分：

  -  记录进程：每10秒获取Windows当前focus的窗口进程名，并将其记录到数据库中，以便用户可以更好地了解自己的应用程序使用情况。 数据库：

  - 数据库使用sqlite3文件，包含两张表： 

    - HourLog：记录每天的日期、时间（以分钟为单位）和应用程序。

      - date INTEGER(YYYYMMDDHH)
      - time INTERER (in minutes)
      - app INTERER

    - AppModels：记录应用程序的名称、文件和类别ID。

      - app INTERER PRIMARY KEY
      - file NVARCHAR
      - CategoryID int NULL DEFAULT 0

    - CategoryModels: 分类ID-名称对照
      
      - CategoryID INTEGER PRIMARY KEY
      - name NVARCHAR
      
      
