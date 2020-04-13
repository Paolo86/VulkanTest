#include "Vk.h"
#include "Window.h"
#include "../Utils/Logger.h"

std::unique_ptr<Vk> Vk::m_instance;

Vk& Vk::Instance()
{
	if (m_instance == nullptr)
		m_instance = std::make_unique<Vk>();

	return *m_instance;
}

Vk::Vk()
{
	//Initialize validation layers
	m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
	m_validationLayersEnabled = true;
}


Vk::~Vk()
{
	Destroy();
}


void Vk::Init()
{
	if (m_validationLayersEnabled && !AreValidationLayersSupported())
	{
		Logger::LogError("A validation layer was not found");
		throw std::runtime_error("Validation layer not found");
	}

	// Optional struct
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = Window::Instance().GetTitle().c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Vulkan 2D";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;


	//Not optional
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t extensionsCount = 0;

	const char** extensions = Window::Instance().GetGLFWExtensions(extensionsCount);


	createInfo.enabledExtensionCount = extensionsCount;
	createInfo.ppEnabledExtensionNames = extensions;

	//Validation layer
	if (m_validationLayersEnabled)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();

	}
	else
	{
		createInfo.enabledLayerCount = 0;

	}

	if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
		Logger::LogError(__FUNCTION__, "Failed to create instance");

	// Populate supported extensions
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
	m_supportedExtensions.resize(extensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, m_supportedExtensions.data());

	//Uncomment to view the list of supported extensions
	for (const auto& extension : m_supportedExtensions)
		Logger::Log(extension.extensionName);
}

void Vk::Destroy()
{
	vkDestroyInstance(m_vkInstance, nullptr);
}


bool Vk::AreValidationLayersSupported()
{
	// Get instance layers
	uint32_t layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
	std::vector<VkLayerProperties> instanceLayers(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, instanceLayers.data());

	//Check against the ones we defined
	for (auto layerName : m_validationLayers)
	{
		bool found = false;

		for (auto instanceLayer : instanceLayers)
		{
			if (strcmp(layerName, instanceLayer.layerName) == 0)
			{
				found = true;
				break;
			}

		}

		//If one layer was not found, return false
		if (!found)
			return false;
	}

	return true;

}
