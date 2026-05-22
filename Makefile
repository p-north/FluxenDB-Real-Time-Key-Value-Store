
# CXX = g++
# CXXFLAGS = -std=c++17 -Wall -pthread -MMD -MP -O2

# SRC_DIR = src
# BUILD_DIR = build

# SERVER_SRC = $(shell find $(SRC_DIR)/server -name "*.cpp")
# CLIENT_SRC = $(shell find $(SRC_DIR)/client -name "*.cpp")

# SERVER_OBJS = $(SERVER_SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
# CLIENT_OBJS = $(CLIENT_SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# SERVER_TARGET = server
# CLIENT_TARGET = client

# # default build BOTH
# all: $(SERVER_TARGET) $(CLIENT_TARGET)

# # create build directory structure
# $(BUILD_DIR):
# 	mkdir -p $(BUILD_DIR)

# # compile rule (supports subfolders)
# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
# 	@mkdir -p $(dir $@)
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# # link server
# $(SERVER_TARGET): $(SERVER_OBJS)
# 	$(CXX) $(CXXFLAGS) $(SERVER_OBJS) -o $(SERVER_TARGET)

# # link client
# $(CLIENT_TARGET): $(CLIENT_OBJS)
# 	$(CXX) $(CXXFLAGS) $(CLIENT_OBJS) -o $(CLIENT_TARGET)

# clean:
# 	rm -rf $(BUILD_DIR) $(SERVER_TARGET) $(CLIENT_TARGET)

# rebuild: clean all

# run-server: $(SERVER_TARGET)
# 	./$(SERVER_TARGET)

# run-client: $(CLIENT_TARGET)
# 	./$(CLIENT_TARGET)