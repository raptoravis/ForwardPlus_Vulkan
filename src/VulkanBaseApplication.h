#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>
#include <random>

#include "VDeleter.h"
#include "camera.h"

// debug validation layers
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
//const bool enableValidationLayers = true;
const bool enableValidationLayers = false;
#endif

// debug report funcs definitions
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);


// const variables
extern const int WIDTH;
extern const int HEIGHT;

#define SIBENIK 0
#define SPONZA 0
#define CRYTEC_SPONZA 1

#if SIBENIK
const std::string MODEL_BASE_DIR = "../src/models/sibenik/";
const std::string MODEL_PATH = MODEL_BASE_DIR + "sibenik.obj";
#elif SPONZA
const std::string MODEL_BASE_DIR = "../src/models/sponza/";
const std::string MODEL_PATH = MODEL_BASE_DIR + "sponza.obj";
#elif CRYTEC_SPONZA
const std::string MODEL_BASE_DIR = "../src/models/crytek-sponza/";
const std::string MODEL_PATH = MODEL_BASE_DIR + "sponza.obj";
#endif

const std::string TEXTURE_COLOR_PATH = MODEL_BASE_DIR + "blank.png";
const std::string TEXTURE_NORM_PATH = MODEL_BASE_DIR + "blank.png";
const std::vector<std::string> TEXTURES_PATH = { TEXTURE_COLOR_PATH ,TEXTURE_NORM_PATH };


const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};


const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, normal);
		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
	}

};


/************************************************************/
//			Base Class for Vulkan Application
/************************************************************/
class VulkanBaseApplication {
public:
	void run();

	// clean up resources
	~VulkanBaseApplication();

private:
	GLFWwindow* window;

	VDeleter<VkInstance> instance{ vkDestroyInstance };
	VDeleter<VkDebugReportCallbackEXT> callback{ instance, DestroyDebugReportCallbackEXT };
	VDeleter<VkSurfaceKHR> surface{ instance, vkDestroySurfaceKHR };

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VDeleter<VkDevice> device{ vkDestroyDevice };

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	// Swap chain related
	VDeleter<VkSwapchainKHR> swapChain{ device, vkDestroySwapchainKHR };
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	// swap chain image views
	std::vector<VkImageView> swapChainImageViews;
	// swap chian frame buffers
	std::vector<VDeleter<VkFramebuffer>> swapChainFramebuffers;

	// Pipeline layout
	VDeleter<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };
	VDeleter<VkPipelineLayout> computePipelineLayout = { device, vkDestroyPipelineLayout };

	// Descriptor pool
	VDeleter<VkDescriptorPool> descriptorPool{ device, vkDestroyDescriptorPool };

	// Descriptor set layout and descriptor set
	VDeleter<VkDescriptorSetLayout> descriptorSetLayout{ device, vkDestroyDescriptorSetLayout };
	VkDescriptorSet descriptorSet;

	// Render pass
	VDeleter<VkRenderPass> renderPass{ device, vkDestroyRenderPass };

	// Command pool
	VDeleter<VkCommandPool> commandPool{ device, vkDestroyCommandPool };

	// Semaphores
	VDeleter<VkSemaphore> imageAvailableSemaphore{ device, vkDestroySemaphore };
	VDeleter<VkSemaphore> renderFinishedSemaphore{ device, vkDestroySemaphore };

	// fence
	VDeleter<VkFence> fence{ device, vkDestroyFence };

	// shader modules
	std::vector<VDeleter<VkShaderModule>> shaderModules;

	// Command buffers
	struct CommandBuffers {
		std::vector<VkCommandBuffer> display;
		VkCommandBuffer compute;
		VkCommandBuffer frustum;
	} cmdBuffers;

	struct ShaderStages {
		VkPipelineShaderStageCreateInfo vs;
		VkPipelineShaderStageCreateInfo fs;
		VkPipelineShaderStageCreateInfo vs_axis;
		VkPipelineShaderStageCreateInfo fs_axis;
		VkPipelineShaderStageCreateInfo fs_quad;
		VkPipelineShaderStageCreateInfo csFrustum;
		VkPipelineShaderStageCreateInfo csLightList;
	} shaderStage;


	// Pipeline(s)
	struct Pipelines {
		VkPipeline graphics; // base pipeline
		VkPipeline axis; // axis pipeline
		VkPipeline quad; // quad pipeline
		VkPipeline computeLightList; // compute light list pipeline
		VkPipeline computeFrustumGrid; // compute Frustum Grid pipeline
		VkPipeline depth;

		void cleanup(VkDevice device) {
			vkDestroyPipeline(device, graphics, nullptr);
			vkDestroyPipeline(device, axis, nullptr);
			vkDestroyPipeline(device, quad, nullptr);
			vkDestroyPipeline(device, computeLightList, nullptr);
			vkDestroyPipeline(device, computeFrustumGrid, nullptr);
			vkDestroyPipeline(device, depth, nullptr);
		}

	} pipelines;

	// Textures
	struct Texture {
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory imageMemory;
		VkSampler sampler;

		void cleanup(VkDevice device) {
			vkDestroyImageView(device, imageView, nullptr);
			vkDestroyImage(device, image, nullptr);
			vkFreeMemory(device, imageMemory, nullptr);
			vkDestroySampler(device, sampler, nullptr);
		}
	};
	std::array<Texture, 2> textures;

	// Vertex/Index buffer struct
	struct VertexBuffer {
		std::vector<Vertex> verticesData;
		VkBuffer buffer;
		VkDeviceMemory mem;
	};

	struct IndexBuffer {
		std::vector<uint32_t> indicesData;
		VkBuffer buffer;
		VkDeviceMemory mem;
	};

	// vulkan buffers
	struct VulkanBuffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
		//VkDescriptorBufferInfo descriptor;
		VkDeviceSize allocSize;

		void cleanup(VkDevice device) {
			vkDestroyBuffer(device, buffer, nullptr);
			vkFreeMemory(device, memory, nullptr);
		}
	};

	struct Material {
		glm::vec4 ambient; // Ka
		glm::vec4 diffuse; // Kd
		//glm::vec4 specular; // Ks

		float specularPower;
		int useTextureMap;
		int useNormMap;
		int useSpecMap;
		//---------------------16 bytes
	};

	struct Mesh {
		VertexBuffer vertices;
		IndexBuffer indices;

		void cleanup(VkDevice device) {
			vkDestroyBuffer(device, vertices.buffer, nullptr);
			vkFreeMemory(device, vertices.mem, nullptr);
			vkDestroyBuffer(device, indices.buffer, nullptr);
			vkFreeMemory(device, indices.mem, nullptr);
		}
	};

	struct MeshGroup {
		VertexBuffer vertices;
		std::vector<IndexBuffer> indexGroups;
		std::vector<Material> materials;
		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<VulkanBuffer> materialBuffers;
		std::vector<Texture> textureMaps;
		std::vector<Texture> normalMaps;
		std::vector<Texture> specMaps;

		void cleanup(VkDevice device) {
			vkDestroyBuffer(device, vertices.buffer, nullptr);
			vkFreeMemory(device, vertices.mem, nullptr);

			for (auto & indexBuffer : indexGroups) {
				vkDestroyBuffer(device, indexBuffer.buffer, nullptr);
				vkFreeMemory(device, indexBuffer.mem, nullptr);
			}

			for (auto & buffer : materialBuffers) {
				buffer.cleanup(device);
			}

			for (int i = 0; i < materials.size(); ++i) {
				if (materials[i].useNormMap > 0) {
					normalMaps[i].cleanup(device);
				}
				if (materials[i].useTextureMap > 0) {
					textureMaps[i].cleanup(device);
				}
				if (materials[i].useSpecMap > 0) {
					specMaps[i].cleanup(device);
				}
			}

		}
	};

	struct Meshes {
		Mesh scene; // main scene mesh
		Mesh axis; // axis mesh
		Mesh quad; // quad mesh

		MeshGroup meshGroupScene; // meshGroup scene by materials

		void cleanup(VkDevice device) {
			//scene.cleanup(device);
			//axis.cleanup(device);
			//quad.cleanup(device);
			meshGroupScene.cleanup(device);
		}
	} meshs;



	// uniform buffers
	struct UniformBuffers {
		VulkanBuffer vsScene, vsSceneStaging;
		VulkanBuffer csParams, csParamsStaging;
		VulkanBuffer fsParams, fsParamsStaging;

		void cleanup(VkDevice device) {
			vsScene.cleanup(device);
			vsSceneStaging.cleanup(device);
			csParams.cleanup(device);
			csParamsStaging.cleanup(device);
			fsParams.cleanup(device);
			fsParamsStaging.cleanup(device);
		}
	} ubo;

	// storage buffers
	struct StorageBuffers {
		VulkanBuffer lights;
		VulkanBuffer frustums;
		VulkanBuffer lightIndex;
		VulkanBuffer lightGrid;

		void cleanup(VkDevice device) {
			lights.cleanup(device);
			frustums.cleanup(device);
			lightIndex.cleanup(device);
			lightGrid.cleanup(device);
		}
	} sbo;

	// forward plus params
	struct ForwardPlusParams {
		int numLights;
		glm::ivec2 numThreads;
		glm::ivec2 numThreadGroups;
	} fpParams;

	// vs uniform layout
	struct UBO_vsParams {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec4 cameraPos;
	};

	// cs uniform layout
	struct UBO_csParams {
		glm::mat4 viewMat;
		glm::mat4 inverseProj;
		glm::ivec2 screenDimensions;
		glm::ivec2 numThreads;
		int numLights;
		float time;
	};

	// fs uniform layout
	struct UBO_fsParams {
		int numLights;
		float time;
		int debugMode;
		float pad; // fuck
		glm::ivec2 numThreads;
		glm::ivec2 screenDimensions;
	};

	// uniform buffer host data
	struct {
		UBO_vsParams vsParams;
		UBO_csParams csParams;
		UBO_fsParams fsParams;
	} uboHostData;

	// storage buffer object to store lights
#define MAX_NUM_LIGHTS 5000
#define MAX_NUM_LIGHTS_PER_TILE 128
	struct SBO_lights {
		// light information
		struct {
			glm::vec4 beginPos; // beginPos.w = intensity
			glm::vec4 endPos; // endPos.w = radius
			glm::vec4 color; // color.w = t
		} lights[MAX_NUM_LIGHTS];;
	};

#define MAX_NUM_FRUSTRUMS 20000
	struct SBO_frustums {
		// frustum definition
		struct {
			// for each plane, use a vec4 to represent
			// xyz is normal, w is distance
			glm::vec4 planes[4];
		} frustums[MAX_NUM_FRUSTRUMS]; // 800*600 -> 50*40
	};

	// storage buffer host data
	struct {
		SBO_lights lights;
		SBO_frustums frustums;
	} sboHostData;


	// Frame buffer attachment struct
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;

		void cleanup(VkDevice device) {
			vkDestroyImageView(device, view, nullptr);
			vkDestroyImage(device, image, nullptr);
			vkFreeMemory(device, mem, nullptr);
		}
	} depth;

	// Depth prepass
	struct DepthPrepass {
		FrameBufferAttachment depth;
		VkFramebuffer frameBuffer;
		VkRenderPass renderPass;
		VkSampler depthSampler;
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		VkSemaphore semaphore = VK_NULL_HANDLE;
	} depthPrepass;

	/************************************************************/
	//					Function Declaration
	/************************************************************/
	void initWindow();

	void initForwardPlusParams();

	void initVulkan();

	void mainLoop();

	void updateUniformBuffer();

	void drawFrame();

	// Create Vulkan instance
	void createInstance();

	void setupDebugCallback();

	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createSwapChain();

	void createImageViews();

	void createShaders();

	void createGraphicsPipeline();

	void createComputePipeline();

	void createFramebuffers();

	void createCommandPool();

	void createCommandBuffers();

	void createFrustumCommandBuffer();

	void createComputeCommandBuffer();

	void createDepthCommandBuffer();

	void createRenderPass();

	void createDepthRenderPass();

	void createDepthFramebuffer();

	void createVertexBuffer(std::vector<Vertex> & verticesData, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void createIndexBuffer(std::vector<uint32_t> &indicesData, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void createSemaphores();

	// find queue families
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	// get extension that are required by instance
	std::vector<const char*> getRequiredExtensions();

	// check validation layer support
	bool checkValidationLayerSupport();

	// helper func: print extension names
	void printInstanceExtensions();

	// check device extension support
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	// query swap chain support, assign SwapChainSupportDetails struct
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	// swap chain choose format
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	// swap chain, choose present mode
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

	// swap chain, choose swap extent
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	// read shader file from compiled binary file
	static std::vector<char> readFile(const std::string& filename);

	// find memory type
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	// abstracting buffer creation
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	// copy buffer from srcBuffer to dstBuffer
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	// descriptor set layout
	void createDescriptorSetLayout();

	void createLightInfos();

	void createUniformBuffer();

	void createStorageBuffer();

	void initStorageBuffer();

	void createDescriptorPool();

	void createDescriptorSet();

	void createDescriptorSetsForMeshGroup(VkDescriptorSet & descriptorSet, VulkanBuffer & buffer, int useTex, Texture & texMap, int useNorm, Texture & norMap, int useSpec, Texture & specMap);

	void createTextureImage(const std::string& texFilename, VkImage & texImage, VkDeviceMemory & texImageMemory);

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);

	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView & imageView);

	void createTextureImageView(VkImage & textureImage, VkImageView & textureImageView);

	void createTextureSampler(VkSampler & textureSampler);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format);

	void createDepthResources();

	void loadModel(std::vector<Vertex> & vertices, std::vector<uint32_t> & indices, const std::string & modelFilename, const std::string & modelBaseDir, float scale = 1.0f);

	void loadModel(MeshGroup & meshGroup, const std::string & modelFilename, const std::string & modelBaseDir, float scale = 1.0f);

	// load axis info
	void loadAxisInfo();

	// assign vertex and index buffer for mesh
	void createMeshBuffer(Mesh& mesh);

	// load texture quad info
	void loadTextureQuad();

	void prepareTextures();

	void prepareTexture(std::string & texturePath, Texture & texture);


	// tools
	bool isDeviceSuitable(VkPhysicalDevice device);

	VkBufferMemoryBarrier createBufferMemoryBarrier(
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkBuffer buffer, VkDeviceSize bufferSize);

	void createShaderModule(
		const std::vector<char> & code,
		VDeleter<VkShaderModule> & shaderModule);

	VkPipelineShaderStageCreateInfo loadShader(
		std::string fileName, VkShaderStageFlagBits stage,
		int shaderModuleIndex);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void resetTitleAndTiming();

};

// callbacks
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
