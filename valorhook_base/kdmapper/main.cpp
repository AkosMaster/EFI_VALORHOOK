#include "kdmapper.hpp"
#include "efi_driver.hpp"

int main(const int argc, char** argv)
{

	if (argc != 2 || std::filesystem::path(argv[1]).extension().string().compare(".sys"))
	{
		std::cout << "[-] Incorrect usage" << std::endl;
		return -1;
	}

	const std::string driver_path = argv[1];

	if (!std::filesystem::exists(driver_path))
	{
		std::cout << "[-] File " << driver_path << " doesn't exist" << std::endl;
		return -1;
	}
	
	HANDLE iqvw64e_device_handle = nullptr; // dummy handle because I am lazy piece of shit

	bool status = efi_driver::Init();
	if (!status)
	{
		std::cout << "[-] Failed to init driver" << std::endl;
		return -1;
	}

	if (!kdmapper::MapDriver(iqvw64e_device_handle, driver_path))
	{
		std::cout << "[-] Failed to map " << driver_path << std::endl;
		return -1;
	}

	std::cout << "[+] success" << std::endl;

	/*
	bool status = efi_driver::Init();
	if (!status)
	{
		std::cout << "[-] Failed to init driver" << std::endl;
		return -1;
	}

	int image_size = 100;
	void* base_source = VirtualAlloc(nullptr, image_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	memcpy(base_source, "MemCpy test successful", image_size);

	void* base_dest = VirtualAlloc(nullptr, image_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	uint64_t kernel_image_base = efi_driver::AllocatePool(nt::NonPagedPoolMustSucceed, 0x1000);
	std::cout << reinterpret_cast<void*>(kernel_image_base) << std::endl;
	
	if (!kernel_image_base) {
		return -1;
	}

	efi_driver::WriteMemory(kernel_image_base, base_source, image_size);
	efi_driver::ReadMemory(kernel_image_base, base_dest, image_size);

	std::cout << (char*)base_dest << std::endl;
	//efi_driver::WriteMemory(kernel_image_base, base_source, image_size - 10);
	//efi_driver::FreePool(kernel_image_base);

	*/
}