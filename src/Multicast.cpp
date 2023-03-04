#include "Multicast.h"
#include "NewProjectiles.h"
#include "AutoAim.h"
#include "Emittors.h"
#include "Following.h"
#include "Targeting.h"

namespace ManyProjs
{
	// Stored in Json
	enum class Shape : uint32_t
	{
		Single,
		HorizontalLine,
		VerticalLine,
		Circle,
		HalfCircle,
		FillSquare,
		FillSphere,

		Total
	};
	static constexpr Shape Shape__DEFAULT = Shape::Single;

	// Stored in Json
	enum class LaunchDir : uint32_t
	{
		Parallel,
		ToSight,
		ToCenter,
		FromCenter,
		AutoAim,

		Total
	};
	static constexpr LaunchDir LaunchDir__DEFAULT = LaunchDir::Parallel;

	// Stored in Json
	enum class SoundType : uint32_t
	{
		Every,
		Single,
		None,

		Total
	};
	static constexpr SoundType SoundType__DEFAULT = SoundType::Every;

	namespace Data
	{
		using Targeting::AimTargetType;
		using Targeting::detection_angle_t;

		class JsonStorage
		{

		public:
			// A part of JsonDataItem
			struct BitData
			{
				uint32_t normalDependsX: 1;                            // 0:00
				LaunchDir rot: 3;                                      // 0:01 -- default
				SoundType sound: 2;                                    // 0:04 -- default
				Shape shape: 3;                                        // 0:06 -- default
				uint32_t count: 8;                                     // 0:09 -- default
				uint32_t size: 15;                                     // 0:11 -- shape not Single. Do not checked in json schema
				AimTargetType target: 2;                               // 1:00
				uint32_t detection_angle : detection_angle_t::SIZEOF;  // 1:02 - for cursor target (0..90)
				uint32_t unused: 23;								   // 1:09
			};
			static_assert(sizeof(BitData) == 0x8);

			struct SpellData
			{
				uint32_t spellID;
			};

			struct ArrowData
			{
				uint32_t weapID; // -- default. -1 = Current
				uint32_t arrowID; // -- default. 0 = unset, -1 = Current
			};

			// Stored in Json, queried only at creation
			struct JsonDataItem
			{
				ProjectileRot rot_rnd;                                 // 00
				ProjectileRot rot_delta;                               // 08 -- rot Parallel
				BitData bitdata;                                       // 10
				RE::NiPoint3 pos;                                      // 18
				RE::NiPoint3 pos_rnd;                                  // 24
				RE::NiPoint3 normal;                                   // 30
				std::variant<SpellData, ArrowData> data;               //
				NewProjType newtypes;                                  //
			};
			static_assert(sizeof(JsonDataItem) == 0x54);

		private:
			struct MapEntry
			{
				std::vector<JsonDataItem> spawnData;
				JsonDataItem def;
			};
			//static_assert(sizeof(MapEntry) == 0x74);

			using Map_t = std::unordered_map<uint32_t, MapEntry>;

			// init fields that could be overriden in spawnData
			static void init_defaults_data(const Json::Value& item, JsonDataItem& data)
			{
				data.bitdata.shape = parse_enum_ifIsMember<Shape::Total>(item, "shape");
				data.bitdata.rot =
					parse_enum_ifIsMember<LaunchDir::Total>(item, "rotation");
				data.bitdata.sound =
					parse_enum_ifIsMember<SoundType::Total>(item, "sound");
				data.bitdata.count = parse_enum_ifIsMember<(uint32_t)0>(item, "count");

				if (item.isMember("spellID")) {
					auto spellID = item["spellID"].asString();
					data.data = SpellData{};
					auto& spelldata = std::get<SpellData>(data.data);
					if (spellID == "Current")
						spelldata.spellID = (uint32_t)-1;
					else
						spelldata.spellID = JsonUtils::get_formid(spellID);
				} else if (item.isMember("weapID")) {
					data.data = ArrowData{};
					auto& arrowdata = std::get<ArrowData>(data.data);

					auto weapID = item["weapID"].asString();
					if (weapID == "Current")
						arrowdata.weapID = (uint32_t)-1;
					else
						arrowdata.weapID = JsonUtils::get_formid(weapID);

					if (item.isMember("arrowID")) {
						auto arrowID = item["arrowID"].asString();
						if (arrowID == "Current")
							arrowdata.arrowID = (uint32_t)-1;
						else
							arrowdata.arrowID = JsonUtils::get_formid(arrowID);
					}
				} else {
					data.data = SpellData{};
				}

				if (data.bitdata.rot == LaunchDir::AutoAim) {
					data.bitdata.target = Targeting::Data::read_json_entry(item);
					data.bitdata.detection_angle = Targeting::Data::read_detectionAngle(item, data.bitdata.target);
				}

				data.newtypes.init(item);
			}

			// init both default & normal entries
			static void read_json_entry(const Json::Value& item, uint32_t formid)
			{
				auto it = map.insert({ formid, {} });
				JsonDataItem& defaults = (*it.first).second.def;

				init_defaults_data(item, defaults);

				// TODO: init rest data if size == 1

				auto readOrDefault3 = [](const Json::Value& json_spawnGroup,
										  std::string_view field_name) -> RE::NiPoint3 {
					if (json_spawnGroup.isMember(field_name.data()))
						return JsonUtils::get_point(json_spawnGroup[field_name.data()]);
					else
						return { 0, 0, 0 };
				};

				auto readOrDefault2 = [](const Json::Value& json_spawnGroup,
										  std::string_view field_name) -> ProjectileRot {
					if (json_spawnGroup.isMember(field_name.data()))
						return JsonUtils::get_point2(json_spawnGroup[field_name.data()]);
					else
						return { 0, 0 };
				};

				auto& spawnData = (*it.first).second.spawnData;
				const auto& json_spawnData = item["spawnData"];
				for (uint32_t i = 0; i < json_spawnData.size(); i++) {
					using JsonUtils::get_point;
					using JsonUtils::get_point2;

					auto& json_spawnGroup = json_spawnData[i];
					JsonDataItem data;
					init_defaults_data(json_spawnGroup, data);

					data.pos = readOrDefault3(json_spawnGroup, "pos"sv);
					data.pos_rnd = readOrDefault3(json_spawnGroup, "posRnd"sv);
					data.rot_rnd = readOrDefault2(json_spawnGroup, "rotRnd"sv);
					data.rot_delta = readOrDefault2(json_spawnGroup, "rotDelta"sv);

					if (data.bitdata.shape != Shape::Single) {
						data.bitdata.size =
							parse_enum_ifIsMember<uint32_t(0)>(json_spawnGroup, "size"sv);
					} else {
						data.bitdata.size = 0;
					}

					data.normal = readOrDefault3(json_spawnGroup, "normal"sv);
					if (data.normal.SqrLength() == 0) {
						data.normal = { 0, 1, 0 };
					}

					data.bitdata.normalDependsX = !json_spawnGroup.isMember("normal") ||
					                              !json_spawnGroup.isMember("xDepends") ||
					                              json_spawnGroup["xDepends"].asBool();

					spawnData.push_back(data);
				}
			}

		public:
			static void init(const Json::Value& multicast, int hex)
			{
				for (auto& formid : multicast.getMemberNames()) {
					read_json_entry(multicast[formid],
						hex | JsonUtils::get_formid(formid));
				}
			}

			static void clear() { map.clear(); }

			static inline Map_t map;
		};
	}

	namespace Casting
	{
		using get_rot_t = std::function<ProjectileRot(const RE::NiPoint3& cur_pos)>;

		template <typename T, T unset, T def>
		T read_default_impl(T data, T data_def)
		{
			T ans = data;
			if (ans == unset)
				ans = data_def;
			if (ans == unset)
				ans = def;
			return ans;
		}

		template <auto unset, auto def>
		auto read_default(decltype(unset) data, decltype(unset) data_def)
		{
			return read_default_impl<decltype(unset), unset, def>(data, data_def);
		}

		RE::NiPoint3 rotate(RE::NiPoint3 normal, ProjectileRot parallel_rot,
			bool dependsX)
		{
			RE::NiMatrix3 M;
			M.EulerAnglesToAxesZXY(dependsX ? parallel_rot.x : 0, 0, parallel_rot.z);
			return M * normal;
		}

		namespace Rotation
		{
			auto rot_at(RE::NiPoint3 dir)
			{
				ProjectileRot rot;
				auto len = dir.Unitize();
				if (len == 0) {
					rot = { 0, 0 };
				} else {
					float polar_angle =
						_generic_foo_<68820, float(RE::NiPoint3 * p)>::eval(
							&dir);  // SkyrimSE.exe+c51f70
					rot = { -asin(dir.z), polar_angle };
				}

				return rot;
			}

			auto rot_at(const RE::NiPoint3& from, const RE::NiPoint3& to)
			{
				return rot_at(to - from);
			}

			float add_rot_x(float val, float d)
			{
				const float PI = 3.1415926f;
				// -pi/2..pi/2
				d = d * PI / 180.0f;
				val += d;
				val = std::max(val, -PI / 2);
				val = std::min(val, PI / 2);
				return val;
			}

			float add_rot_z(float val, float d)
			{
				const float PI = 3.1415926f;
				// -pi/2..pi/2
				d = d * PI / 180.0f;
				val += d;
				while (val < 0) val += 2 * PI;
				while (val > 2 * PI) val -= 2 * PI;
				return val;
			}

			auto add_rot(ProjectileRot rot, ProjectileRot delta)
			{
				rot.x = add_rot_x(rot.x, delta.x);
				rot.z = add_rot_z(rot.z, delta.z);
				return rot;
			}

			auto add_rot_rnd(ProjectileRot rot, ProjectileRot rnd)
			{
				if (rnd.x == 0 && rnd.z == 0)
					return rot;

				rot.x = add_rot_x(rot.x, rnd.x * FenixUtils::random_range(-1.0f, 1.0f));
				rot.z = add_rot_z(rot.z, rnd.z * FenixUtils::random_range(-1.0f, 1.0f));
				return rot;
			}

			auto add_point_rnd(const RE::NiPoint3& rnd)
			{
				using FenixUtils::random_range;

				if (rnd.x == 0 && rnd.y == 0 && rnd.z == 0)
					return RE::NiPoint3{ 0, 0, 0 };

				return RE::NiPoint3{ rnd.x * random_range(-1.0f, 1.0f),
					rnd.y * random_range(-1.0f, 1.0f),
					rnd.z * random_range(-1.0f, 1.0f) };
			}

			auto find_target(RE::TESObjectREFR* origin, Targeting::AimTargetType type, float angle)
			{
				auto target = Targeting::findTarget(origin, { type, angle });
				return target ? Targeting::get_victim_pos(target) : origin->GetPosition() + RE::NiPoint3{ 0, 1, 0 };
			}

			// get point `caster` is looking at
			auto raycast(RE::Actor* caster)
			{
				auto havokWorldScale = RE::bhkWorld::GetWorldScale();
				RE::bhkPickData pick_data;
				RE::NiPoint3 ray_start, ray_end;

				FenixUtils::Actor__get_eye_pos(caster, ray_start, 2);
				ray_end = ray_start + FenixUtils::rotate(20000, caster->data.angle);
				pick_data.rayInput.from = ray_start * havokWorldScale;
				pick_data.rayInput.to = ray_end * havokWorldScale;

				uint32_t collisionFilterInfo = 0;
				caster->GetCollisionFilterInfo(collisionFilterInfo);
				pick_data.rayInput.filterInfo =
					(static_cast<uint32_t>(collisionFilterInfo >> 16) << 16) |
					static_cast<uint32_t>(RE::COL_LAYER::kCharController);

				caster->GetParentCell()->GetbhkWorld()->PickObject(pick_data);
				RE::NiPoint3 hitpos;
				if (pick_data.rayOutput.HasHit()) {
					hitpos = ray_start +
					         (ray_end - ray_start) * pick_data.rayOutput.hitFraction;
				} else {
					hitpos = ray_end;
				}
				return hitpos;
			}
		}

		struct Plane
		{
			RE::NiPoint3 startPos, right_dir, up_dir;
		};

		struct ShapeParams
		{
			float size;
			uint32_t count;
		};

		auto multiCastGroupItem(const get_rot_t& get_rot, RE::NiPoint3 pos,
			const Data::JsonStorage::JsonDataItem& data, const CastData& cast_data,
			bool withSound)
		{
			RE::NiPoint3 rnd_offset = Rotation::add_point_rnd(data.pos_rnd);
			rnd_offset = rotate(rnd_offset, { cast_data.parallel_rot.x, cast_data.parallel_rot.z }, data.bitdata.normalDependsX);

			pos += rnd_offset;

			ProjectileRot new_rot = get_rot(pos);

			new_rot = Rotation::add_rot(new_rot, data.rot_delta);
			new_rot = Rotation::add_rot_rnd(new_rot, data.rot_rnd);

			auto type = cast_data.data.index();
			if (type == 0) {
				// SpellData
				return cast_CustomPos(cast_data.caster,
					std::get<CastData::SpellData>(cast_data.data).spel, pos, new_rot,
					withSound);
			} else {
				// ArrowData
				auto& arrow_data = std::get<CastData::ArrowData>(cast_data.data);
				return cast_CustomPos(cast_data.caster, arrow_data.ammo, arrow_data.weap,
					pos, new_rot);
			}
		}

		static const std::array<std::function<std::vector<RE::NiPoint3>(
									const ShapeParams& shape_params, const Plane& plane)>,
			(size_t)Shape::Total>
			MultiCasters = {

				// Single
				[](ShapeParams shape_params,
					const Plane& plane) -> std::vector<RE::NiPoint3> {
					return std::vector<RE::NiPoint3>(shape_params.count, plane.startPos);
				},

				// HorizontalLine
				[](ShapeParams shape_params,
					const Plane& plane) -> std::vector<RE::NiPoint3> {
					if (shape_params.count == 1) {
						return { plane.startPos };
					}

					std::vector<RE::NiPoint3> ans;
					auto from =
						plane.startPos - plane.right_dir * (shape_params.size * 0.5f);
					float d = shape_params.size / (shape_params.count - 1);
					for (uint32_t i = 0; i < shape_params.count; i++) {
						ans.push_back(from);
						from += plane.right_dir * d;
					}
					return ans;
				},

				// VerticalLine
				[](ShapeParams shape_params,
					const Plane& plane) -> std::vector<RE::NiPoint3> {
					Plane new_plane = plane;
					std::swap(new_plane.right_dir, new_plane.up_dir);
					return MultiCasters[(size_t)Shape::HorizontalLine](shape_params,
						new_plane);
				},

				// Circle
				[](ShapeParams shape_params,
					const Plane& plane) -> std::vector<RE::NiPoint3> {
					std::vector<RE::NiPoint3> ans;
					for (uint32_t i = 0; i < shape_params.count; i++) {
						float alpha = 2 * 3.1415926f / shape_params.count * i;

						auto cur_p = plane.startPos +
				                     plane.right_dir * cos(alpha) * shape_params.size +
				                     plane.up_dir * sin(alpha) * shape_params.size;

						ans.push_back(cur_p);
					}
					return ans;
				},

				// HalfCircle
				[](ShapeParams shape_params,
					const Plane& plane) -> std::vector<RE::NiPoint3> {
					if (shape_params.count == 1) {
						return { plane.startPos };
					}

					std::vector<RE::NiPoint3> ans;
					for (uint32_t i = 0; i < shape_params.count; i++) {
						float alpha = 3.1415926f / (shape_params.count - 1) * i;

						auto cur_p = plane.startPos +
				                     plane.right_dir * cos(alpha) * shape_params.size +
				                     plane.up_dir * sin(alpha) * shape_params.size;

						ans.push_back(cur_p);
					}
					return ans;
				},

				// FillSquare
				[](ShapeParams shape_params,
					const Plane& plane) -> std::vector<RE::NiPoint3> {
					// TODO: faster
					uint32_t h = (uint32_t)ceil(sqrt(shape_params.count));

					if (h == 1) {
						return MultiCasters[(size_t)Shape::HorizontalLine](shape_params,
							plane);
					}

					auto from =
						plane.startPos + plane.up_dir * (shape_params.size * 0.5f);
					float d = shape_params.size / (h - 1);

					ShapeParams params = { shape_params.size, h };
					Plane cur_plane = plane;
					std::vector<RE::NiPoint3> ans;
					while (shape_params.count >= h) {
						cur_plane.startPos = from;
						auto cur_line = MultiCasters[(uint32_t)Shape::HorizontalLine](
							params, cur_plane);
						ans.insert(ans.end(), cur_line.begin(), cur_line.end());
						from -= plane.up_dir * d;
						shape_params.count -= h;
					}

					if (shape_params.count > 0) {
						cur_plane.startPos = from;
						params.count = shape_params.count;
						auto cur_line = MultiCasters[(uint32_t)Shape::HorizontalLine](
							params, cur_plane);
						ans.insert(ans.end(), cur_line.begin(), cur_line.end());
					}

					return ans;
				},

				// FillSphere
				[](ShapeParams, const Plane& plane) -> std::vector<RE::NiPoint3> {
					return { plane.startPos };
				}
			};

		using SpellArrowData = std::variant<Data::JsonStorage::SpellData, Data::JsonStorage::ArrowData>;
		auto read_default(const SpellArrowData& data, const SpellArrowData& data_def) {
			auto data_ind = data.index();

			if (data_ind == 1) {
				// ArrowData
				return data;
			}

			if (data_ind == 0 && std::get<0>(data).spellID != 0) {
				// SpellData, not empty
				return data;
			}

			return data_def;
		}

		void multiCastGroup(const Data::JsonStorage::JsonDataItem& data,
			const Data::JsonStorage::JsonDataItem& data_def, CastData& cast_data)
		{
			auto spellarrow_data = read_default(data.data, data_def.data);

			auto type = spellarrow_data.index();
			if (type == 0) {
				auto& spell_data = std::get<Data::JsonStorage::SpellData>(spellarrow_data);

				auto spell_id = spell_data.spellID;

				if (cast_data.data.index() != 0)
					cast_data.data = CastData::SpellData{};
				if (spell_id != 0 && spell_id != (uint32_t)-1) {
					std::get<CastData::SpellData>(cast_data.data).spel = RE::TESForm::LookupByID<RE::SpellItem>(spell_id);
				}
			} else {
				auto& arrow_data = std::get<Data::JsonStorage::ArrowData>(spellarrow_data);

				auto arrow_id = arrow_data.arrowID;
				auto weap_id = arrow_data.weapID;

				if (cast_data.data.index() != 1)
					cast_data.data = CastData::ArrowData{};
				auto& arrowdata = std::get<CastData::ArrowData>(cast_data.data);
				
				if (weap_id != 0 && weap_id != (uint32_t)-1) {
					arrowdata.weap = RE::TESForm::LookupByID<RE::TESObjectWEAP>(weap_id);
				}
				if (arrow_id != 0 && arrow_id != (uint32_t)-1) {
					arrowdata.ammo = RE::TESForm::LookupByID<RE::TESAmmo>(arrow_id);
				}
			}

			uint32_t count = read_default<(uint32_t)0, (uint32_t)1>(data.bitdata.count,
				data_def.bitdata.count);
			SoundType needsound = read_default<SoundType::Total, SoundType__DEFAULT>(
				data.bitdata.sound, data_def.bitdata.sound);
			LaunchDir rot = read_default<LaunchDir::Total, LaunchDir__DEFAULT>(
				data.bitdata.rot, data_def.bitdata.rot);
			Shape shape = read_default<Shape::Total, Shape__DEFAULT>(data.bitdata.shape,
				data_def.bitdata.shape);
			float size =
				shape == Shape::Single ? 0 : static_cast<float>(data.bitdata.size);
			RE::NiPoint3 cast_dir =
				rotate(data.normal, cast_data.parallel_rot,
				data.bitdata.normalDependsX);
			cast_dir.Unitize();

			RE::NiPoint3 right_dir = RE::NiPoint3(0, 0, -1).UnitCross(cast_dir);
			if (right_dir.SqrLength() == 0)
				right_dir = { 1, 0, 0 };
			RE::NiPoint3 up_dir = right_dir.Cross(cast_dir);

			auto spawn_center =
				*cast_data.startPos +
				rotate(data.pos, cast_data.parallel_rot, data.bitdata.normalDependsX);

			auto points = MultiCasters[(uint32_t)shape]({ size, count },
				{ spawn_center, right_dir, up_dir });

			RE::NiPoint3 arg_point;
			switch (rot) {
			case ManyProjs::LaunchDir::Parallel:
				arg_point = cast_dir;
				break;
			case ManyProjs::LaunchDir::ToSight:
				arg_point = Rotation::raycast(cast_data.caster);
				break;
			case ManyProjs::LaunchDir::AutoAim:
				{
					using Targeting::AimTargetType;
					auto target_type =
						read_default<AimTargetType::Total, AimTargetType::Nearest>(data.bitdata.target, data_def.bitdata.target);
					float angle = 0;
					if (target_type == AimTargetType::Cursor) {
						uint32_t _angle = read_default<(uint32_t)0, (uint32_t)0>(data.bitdata.detection_angle,
							data_def.bitdata.detection_angle);
						angle = Targeting::Data::get_detectionAngle(_angle);
					}
					arg_point = Rotation::find_target(cast_data.caster, target_type, angle);
				}
				break;
			case ManyProjs::LaunchDir::ToCenter:
			case ManyProjs::LaunchDir::FromCenter:
			default:
				arg_point = spawn_center;
				break;
			}
			
			const get_rot_t get_rot = [rot, &arg_point](const RE::NiPoint3& cur_pos) {
				ProjectileRot new_rot;
				switch (rot) {
				case LaunchDir::FromCenter:
					new_rot = Rotation::rot_at(arg_point, cur_pos);
					break;
				case LaunchDir::ToCenter:
					new_rot = Rotation::rot_at(cur_pos, arg_point);
					break;
				case LaunchDir::AutoAim:
				case LaunchDir::ToSight:
					new_rot = Rotation::rot_at(cur_pos, arg_point);
					break;
				case LaunchDir::Parallel:
				default:
					new_rot = Rotation::rot_at(arg_point);
					break;
				}
				return new_rot;
			};

			// SpellData
			if (type == 0 && needsound == SoundType::Single)
				Sounds::play_cast_sound(cast_data.caster,
					std::get<CastData::SpellData>(cast_data.data).spel, spawn_center);
			
			auto read_key = [](uint32_t data_newtypes_field, uint32_t data_def_newtypes_field) {
				return read_default<(uint32_t)0, (uint32_t)0>(data_newtypes_field, data_def_newtypes_field);
			};

			uint32_t key_emitter = read_key(data.newtypes.emitter, data_def.newtypes.emitter);
			uint32_t key_homing = read_key(data.newtypes.homing, data_def.newtypes.homing);
			uint32_t key_follower = read_key(data.newtypes.follower, data_def.newtypes.follower);
			
			bool need_changetype = key_emitter != 0 || key_homing != 0 || key_follower != 0;

			for (uint32_t i = 0; i < points.size(); ++i) {
				auto handle = multiCastGroupItem(get_rot, points[i], data, cast_data,
					needsound == SoundType::Every);

				if (need_changetype) {
					if (auto proj = handle.get().get()) {
						if (key_homing)
							AutoAim::onCreated(proj, key_homing);
					
						if (key_emitter)
							Emitters::onCreated(proj, key_emitter);

						if (key_follower)
							Following::onCreated(proj, key_follower, i);
					}
				}
			}
		}

		auto get_ManyCasted(uint32_t key)
		{
			auto found = Data::JsonStorage::map.find(key);
			return std::make_pair(found != Data::JsonStorage::map.end(), found);
		}

		bool is_ManyCasted(uint32_t key)
		{
			auto found = Data::JsonStorage::map.find(key);
			return found != Data::JsonStorage::map.end();
		}

		void onManyCasted(uint32_t key, CastData& cast_data)
		{
			auto found = get_ManyCasted(key);
			if (!found.first)
				return;

			auto& data = (*found.second).second;

			if (data.spawnData.empty()) {
				// TODO
				return;
			}

			for (const auto& spawn_data : data.spawnData) {
				auto spellarrow_data = read_default(spawn_data.data, data.def.data);
				auto spawn_ind = spellarrow_data.index();
				
				auto data_ind = cast_data.data.index();
				if (data_ind != spawn_ind) {
					if (spawn_ind == 0)
						cast_data.data = CastData::SpellData{};
					else
						cast_data.data = CastData::ArrowData{};
				}

				if (spawn_ind == 0) {
					auto& spell_data = std::get<Data::JsonStorage::SpellData>(spellarrow_data);
					// init current spell
					auto& spelldata = std::get<CastData::SpellData>(cast_data.data);

					if (spell_data.spellID && spell_data.spellID != (uint32_t)-1) {
						spelldata.spel = RE::TESForm::LookupByID<RE::SpellItem>(spell_data.spellID);
					}
				}
				if (spawn_ind == 1) {
					auto& arrow_data = std::get<Data::JsonStorage::ArrowData>(spellarrow_data);
					// init current ammo & weapon
					auto& arrowdata = std::get<CastData::ArrowData>(cast_data.data);

					if (arrow_data.arrowID != 0 && arrow_data.arrowID != (uint32_t)-1)
						arrowdata.ammo = RE::TESForm::LookupByID<RE::TESAmmo>(arrow_data.arrowID);

					if (arrow_data.weapID && arrow_data.weapID != (uint32_t)-1)
						arrowdata.weap = RE::TESForm::LookupByID<RE::TESObjectWEAP>(arrow_data.weapID);
				}

				multiCastGroup(spawn_data, data.def, cast_data);
			}
		}

		// Perform multicast with key=(spell or ammo).proj.formid
		void onManyCasted(CastData& cast_data)
		{
			auto ind = cast_data.data.index();
			if (ind == 0) {
				// SpellData
				auto mgef = FenixUtils::getAVEffectSetting(
					std::get<CastData::SpellData>(cast_data.data).spel);
				if (!mgef)
					return;

				auto bproj = mgef->data.projectileBase;
				if (!bproj)
					return;

				auto formID = bproj->formID;

				return onManyCasted(formID, cast_data);
			}

			if (ind == 1) {
				return onManyCasted(std::get<CastData::ArrowData>(cast_data.data)
										.ammo->data.projectile->formID,
					cast_data);
			}
		}

		// Used for magic hook. Perform multicast with spell = magic_caster's current spell.
		void onManyCasted(RE::MagicCaster* magic_caster, RE::Actor* caster,
			RE::NiPoint3* startPos, ProjectileRot parallel_rot)
		{
			if (!magic_caster->currentSpell)
				return;

			auto spel = magic_caster->currentSpell->As<RE::SpellItem>();
			if (!spel)
				return;

			CastData cast_data = { caster, parallel_rot, startPos, CastData::SpellData{ spel } };
			return onManyCasted(cast_data);
		}

		// Used for arrow hook. Perform multicast with arrow = actor's currect 
		void onManyCasted(RE::Actor* shooter, RE::NiPoint3* startPos,
			ProjectileRot parallel_rot, RE::TESObjectWEAP* weap, RE::TESAmmo* ammo)
		{
			CastData cast_data = { shooter, parallel_rot, startPos,
				CastData::ArrowData{ weap, ammo } };
			return onManyCasted(cast_data);
		}
	}

	namespace Hooks
	{
		// Spawn many projectiles
		class ManyProjsHook
		{
		public:
			static void Hook()
			{
				_castProjectile = SKSE::GetTrampoline().write_call<5>(REL::ID(33670).address() + 0x575,
					castProjectile);  // SkyrimSE.exe+5504F5
			}

		private:
			static bool castProjectile(RE::MagicCaster* a, RE::BGSProjectile* bproj, RE::Actor* a_char, RE::CombatController* a4,
				RE::NiPoint3* startPos, float rotationZ, float rotationX, uint32_t area, void* a9)
			{
				auto found = Casting::get_ManyCasted(bproj->formID);
				if (!found.first)
					return _castProjectile(a, bproj, a_char, a4, startPos, rotationZ, rotationX, area, a9);

				Casting::onManyCasted(a, a_char, startPos, { rotationX, rotationZ });

				return true;
			}

			static inline REL::Relocation<decltype(castProjectile)> _castProjectile;
		};

		struct Projectile__LaunchData
		{
			void* vftable_LaunchData_0;
			RE::NiPoint3 startPos;
			RE::NiPoint3 Point_14;
			RE::BGSProjectile* projectile;
			RE::TESObjectREFR* source;
			RE::CombatController* combatController;
			RE::TESObjectWEAP* weap;
			RE::TESAmmo* overwriteAmmo;
			float rotationZ;
			float rotationX;
			void* field_50;
			RE::TESObjectREFR* target;
			std::pair<float, float> drawn_time;
			RE::TESObjectCELL* cell;
			RE::MagicItem* CastItem;
			RE::MagicSystem::CastingSource castingSource;
			RE::EnchantmentItem* ammoEnchantment;
			RE::AlchemyItem* poison;
			uint32_t area;
			float Effectiveness;
			float scale;
			char field_9C;
			char field_9D;
			char field_9E;
			char field_9F;
			char field_A0;
			char field_A1;
			char field_A2;
			char field_A3;
			char gapA4[4];
		};

		// Spawn many arrows
		class SetNewTypeHook
		{
		public:
			static void Hook()
			{
				FenixUtils::writebytes<17693, 0xefa>("\x0F\x1F\x80\x00\x00\x00\x00"sv);

				_CreateProjectile = SKSE::GetTrampoline().write_call<5>(REL::ID(17693).address() + 0xe82,
					CreateProjectile);  // SkyrimSE.exe+2360C2 -- TESObjectWEAP::Fire_140235240
			}

		private:
			static uint32_t* CreateProjectile(uint32_t* handle, Projectile__LaunchData* ldata)
			{
				auto bproj = ldata->projectile;

				auto found = Casting::get_ManyCasted(bproj->formID);
				if (!found.first)
					return _CreateProjectile(handle, ldata);

				Casting::onManyCasted(ldata->source->As<RE::Actor>(), &ldata->startPos,
					ProjectileRot{ ldata->rotationX, ldata->rotationZ }, ldata->weap, ldata->overwriteAmmo);

				*handle = 0;

				return handle;
			}

			static inline REL::Relocation<decltype(CreateProjectile)> _CreateProjectile;
		};
	}

	void forget() { Data::JsonStorage::clear(); }

	void add_mod(const Json::Value& multicast, int hex) { Data::JsonStorage::init(multicast, hex); }

	void install() {
		Hooks::ManyProjsHook::Hook();
		Hooks::SetNewTypeHook::Hook();
	}
}
