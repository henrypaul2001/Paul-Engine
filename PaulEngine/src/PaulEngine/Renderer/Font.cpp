#include "pepch.h"
#include "Font.h"

#include "PaulEngine/Renderer/MSDFData.h"

#include "Texture.h"

namespace PaulEngine
{
	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs, const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height) {
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

		Ref<Texture2D> texture = Texture2D::Create(spec);
		texture->SetData(Buffer((void*)bitmap.pixels, bitmap.width * bitmap.height * 3));
		return texture;
	}

	Font::Font(const std::filesystem::path& filepath) : m_Data(new MSDFData()) {
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		PE_CORE_ASSERT(ft, "Freetype is not initialized");
		
		std::string filestring = filepath.string();

		msdfgen::FontHandle* font = msdfgen::loadFont(ft, filestring.c_str());
		if (!font) {
			PE_CORE_ERROR("Failed to load font at path: {0}", filestring);
			return;
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
		m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
		int count = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
		PE_CORE_INFO("Loaded {0}/{1} glyphs from font at path: {2}", count, charset.size(), filestring);

		double emSize = 40.0;
		msdf_atlas::TightAtlasPacker atlasPacker;
		// atlasPacker.setDimensionsConstraint();
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0);
		atlasPacker.setScale(emSize);
		int remaining = atlasPacker.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());
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
			msdf_atlas::Workload([&glyphs = m_Data->Glyphs, &colouringSeed](int i, int threadNo) -> bool {
				unsigned long long glyphSeed = (LCG_MULTIPLIER * (colouringSeed ^ i) + LCG_INCREMENT) * !!colouringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
				}, m_Data->Glyphs.size()).finish(THREAD_COUNT);
		}
		else {
			unsigned long long glyphSeed = colouringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : m_Data->Glyphs) {
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}

		m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", (float)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);
	}

	Font::~Font()
	{
		delete m_Data;
	}

	Ref<Font> Font::GetDefault()
	{
		static Ref<Font> DefaultFont;
		if (!DefaultFont) {
			DefaultFont = CreateRef<Font>("assets/fonts/Open_Sans/static/OpenSans-Regular.ttf");
		}
		return DefaultFont;
	}
}