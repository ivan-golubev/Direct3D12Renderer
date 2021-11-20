#include <iostream>
#include <exception>
import D3D12Renderer;
import ErrorHandling;

using awesome::renderer::D3D12Renderer;
using awesome::errorhandling::ComException;

int main()
{
    try {
        std::shared_ptr<D3D12Renderer> renderer{ std::make_shared<D3D12Renderer>() };
        renderer->Init();

        std::cout << "Hello awesome d3d12 renderer" << std::endl;
    }
    catch (ComException const & e) {
        std::cerr << "Caught ComException with message: " << e.whatString() << std::endl;
    }
    catch (std::exception const & e) {
        std::cerr << e.what() << std::endl;
    }
}
