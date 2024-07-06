#
# Created by loumouli on 19/12/23.
#

BUILD_FOLDER = build
EXEC = ft_traceroute

all:
	cmake -S . -B $(BUILD_FOLDER)
	cmake --build $(BUILD_FOLDER)
clean:
	find $(BUILD_FOLDER) -type f ! -name "$(EXEC)" -delete
	find $(BUILD_FOLDER) -type d ! -path "$(BUILD_FOLDER)" -exec rm -rf {} +

fclean:
	rm -rf $(BUILD_FOLDER)

re:			fclean all

.PHONY: all clean fclean re

.NOTPARALLEL: fclean
