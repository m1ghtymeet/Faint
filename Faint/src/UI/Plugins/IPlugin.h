#pragma once

enum class EPluginExecutionContext {
	PANEL
};

class IPlugin {
public:
	/**
	* Execute the plugin behaviour
	* @param p_context
	*/
	virtual void Execute(EPluginExecutionContext p_context) = 0;

	/* Feel free to store any data you want here */
	void* userData = nullptr;
};