# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c         # list the source files of this component
         rgb_led.c
         wifi_app.c
         http_server.c
         temp_rh_meas.c
         app_nvs.c
         wifi_reset_button.c
         sntp_time_sync.c
         mqtt_bk.c
         #uart_bk.c
         UART/uart_bk_WORKING.c
         tasks_common.c
    INCLUDE_DIRS "."   # optional, add here public include directories
    EMBED_FILES webpage/app.css 
    			webpage/app.js 
    			webpage/favicon.ico 
    			webpage/index.html 
    			webpage/jquery-3.3.1.min.js
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
