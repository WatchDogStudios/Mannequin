#include <TestFramework/Framework/TestFramework.h>

#include <Foundation/Basics.h>
#include <Foundation/Basics/Assert.h>
#include <Foundation/Types/TypeTraits.h>
#include <Foundation/Types/Types.h>

#include <Foundation/Containers/Deque.h>
#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>

#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>

#include <Foundation/Math/Declarations.h>

#include <Core/Graphics/Camera.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererFoundation/Device/SwapChain.h>
#include <Texture/Image/Image.h>
#include <Texture/Image/ImageConversion.h>
#include <Texture/Image/ImageUtils.h>

using nsMathTestType = float;

using nsVec2T = nsVec2Template<nsMathTestType>;                     ///< This is only for testing purposes
using nsVec3T = nsVec3Template<nsMathTestType>;                     ///< This is only for testing purposes
using nsVec4T = nsVec4Template<nsMathTestType>;                     ///< This is only for testing purposes
using nsMat3T = nsMat3Template<nsMathTestType>;                     ///< This is only for testing purposes
using nsMat4T = nsMat4Template<nsMathTestType>;                     ///< This is only for testing purposes
using nsQuatT = nsQuatTemplate<nsMathTestType>;                     ///< This is only for testing purposes
using nsPlaneT = nsPlaneTemplate<nsMathTestType>;                   ///< This is only for testing purposes
using nsBoundingBoxT = nsBoundingBoxTemplate<nsMathTestType>;       ///< This is only for testing purposes
using nsBoundingSphereT = nsBoundingSphereTemplate<nsMathTestType>; ///< This is only for testing purposes
