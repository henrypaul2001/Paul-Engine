#include "pepch.h"

#include "FontImporter.h"

#include "PaulEngine/Project/Project.h"
#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Renderer/Texture.h"

#include "PaulEngine/Renderer/MSDFData.h"
#include "PaulEngine/Asset/AssetManager.h"

namespace PaulEngine
{
	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs, const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height, bool persistent = false) {
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(8);
		generator.generate(glyphs.data(), (int)glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

		TextureSpecification spec;
		spec.Width = bitmap.width;
		spec.Height = bitmap.height;
		spec.Format = ImageFormat::RGB8;
		spec.GenerateMips = false;

		Ref<Texture2D> texture = AssetManager::CreateAsset<Texture2D>(persistent, spec);
		texture->SetData(Buffer((void*)bitmap.pixels, bitmap.width * bitmap.height * 3));
		return texture;
	}

	Ref<Font> FontImporter::ImportFont(AssetHandle handle, const AssetMetadata& metadata)
	{
		PE_PROFILE_FUNCTION();
		Ref<Font> font = LoadFont(Project::GetAssetDirectory() / metadata.FilePath, metadata.Persistent);
		font->Handle = handle;
		return font;
	}

	Ref<Font> FontImporter::LoadFont(const std::filesystem::path& filepath, const bool persistentTextureAtlas)
	{
		PE_PROFILE_FUNCTION();

		Ref<Font> fontAsset = CreateRef<Font>();
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		PE_CORE_ASSERT(ft, "Freetype is not initialized");

		std::string filestring = filepath.string();

		msdfgen::FontHandle* font = msdfgen::loadFont(ft, filestring.c_str());
		if (!font) {
			PE_CORE_ERROR("Failed to load font at path: {0}", filestring);
			return nullptr;
		}

		struct CharsetRange {
			uint32_t Begin, End;
		};

		static const CharsetRange charsetRanges[] = { 0x0020, 0x00FF };

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges) {
			for (uint32_t c = range.Begin; c <= range.End; c++) {
				charset.add(c);
			}
		}

		double fontScale = 1.0;
		fontAsset->m_Data->FontGeometry = msdf_atlas::FontGeometry(&fontAsset->m_Data->Glyphs);
		int count = fontAsset->m_Data->FontGeometry.loadCharset(font, fontScale, charset);
		PE_CORE_INFO("Loaded {0}/{1} glyphs from font at path: {2}", count, charset.size(), filestring);

		double emSize = 40.0;
		msdf_atlas::TightAtlasPacker atlasPacker;
		// atlasPacker.setDimensionsConstraint();
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0);
		atlasPacker.setScale(emSize);
		int remaining = atlasPacker.pack(fontAsset->m_Data->Glyphs.data(), (int)fontAsset->m_Data->Glyphs.size());
		PE_CORE_ASSERT(remaining == 0, "Atlas pack unsuccessful");

		int width, height;
		atlasPacker.getDimensions(width, height);
		emSize = atlasPacker.getScale();

#define DEFAULT_ANGLE_THRESHOLD 3.0
#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define THREAD_COUNT 8

		uint64_t colouringSeed = 0;
		bool expensiveColouring = false;
		if (expensiveColouring) {
			msdf_atlas::Workload([&glyphs = fontAsset->m_Data->Glyphs, &colouringSeed](int i, int threadNo) -> bool {
				unsigned long long glyphSeed = (LCG_MULTIPLIER * (colouringSeed ^ i) + LCG_INCREMENT) * !!colouringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
				}, fontAsset->m_Data->Glyphs.size()).finish(THREAD_COUNT);
		}
		else {
			unsigned long long glyphSeed = colouringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : fontAsset->m_Data->Glyphs) {
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}

		fontAsset->m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, fontAsset->m_Data->Glyphs, fontAsset->m_Data->FontGeometry, width, height, persistentTextureAtlas);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);

		return fontAsset;
	}
}