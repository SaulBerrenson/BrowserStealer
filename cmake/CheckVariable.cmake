##################################################################################################################################
#-------------------------------------------------------Function checking and show variables-------------------------------------------#
##################################################################################################################################
function(check_variable glob_var)

if(NOT DEFINED  ${glob_var})
      if (NOT $ENV{${glob_var}} STREQUAL "")
        set (${glob_var} $ENV{${glob_var}}) 
	message(STATUS "|-> Environment Found  ${glob_var} - ${${glob_var}}")         
    else() 
        message(WARNING "|-> Local Variable (${glob_var}) Not Found")
    endif()          
endif()     
    
if (NOT ${glob_var} STREQUAL "")
    message(STATUS "|->  ${glob_var} - ${${glob_var}}")
else()
    message(WARNING "|->  ${glob_var} - NOT FOUND")
endif()
        
endfunction(check_variable)


function(check_option name_option description default_value)

if(NOT DEFINED  ${name_option})
      if (NOT $ENV{${name_option}} STREQUAL "")
        option(${name_option} ${description} $ENV{${name_option}}) 
	message(STATUS "|-> Environment Found  ${name_option} - ${${name_option}}")         
    else() 
        message(STATUS "|-> Option will set ${default_value} (${name_option})")
	option(${name_option} ${description} ${default_value})
    endif()          
endif()     
    
if (NOT ${name_option} STREQUAL "")
    message(STATUS "|->  ${name_option} - ${${name_option}}")
endif()
        
endfunction(check_option)
##################################################################################################################################