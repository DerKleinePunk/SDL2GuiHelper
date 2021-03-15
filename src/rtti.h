#pragma once
#ifndef SDL2GUITEST_SRC_FRAMEWORK_RTTI
#define SDL2GUITEST_SRC_FRAMEWORK_RTTI

#define RTTI_TYPE const std::type_info&
#define RTTI_GET_TYPE(type) typeid(type)

#define DECLARE_TYPE_NAME(Type) \
		public: \
			static const std::string& getClassTypeName() { static std::string type = #Type; return type; } \
			/** Get type name as string */ \
			const std::string& getTypeName() const { return getClassTypeName(); }

#define RTTI_BASE(BaseType) \
		public: \
			typedef BaseType RTTIBase; \
			DECLARE_TYPE_NAME(BaseType) \
			/** Compare with selected type */ \
			virtual bool isType(RTTI_TYPE _type) const { return RTTI_GET_TYPE(BaseType) == _type; } \
			/** Compare with selected type */ \
			template<typename Type> bool isType() const { return isType(RTTI_GET_TYPE(Type)); } \
			/** Try to cast pointer to selected type. \
										@param _throw If true throw exception when casting in wrong type, else return nullptr \
																				*/ \
			template<typename Type> Type* castType(bool _throw = true) \
			{ \
				if (this->isType<Type>()) return static_cast<Type*>(this); \
				/*MYGUI_ASSERT(!_throw, "Error cast type '" << this->getTypeName() << "' to type '" << Type::getClassTypeName() << "' .")*/ \
				return nullptr; \
			} \
			/** Try to cast pointer to selected type. \
										@param _throw If true throw exception when casting in wrong type, else return nullptr \
																				*/ \
			template<typename Type> const Type* castType(bool _throw = true) const \
			{ \
				if (this->isType<Type>()) return static_cast<Type*>(this); \
				/*MYGUI_ASSERT(!_throw, "Error cast type '" << this->getTypeName() << "' to type '" << Type::getClassTypeName() << "' .")*/ \
				return nullptr; \
			}

#define RTTI_DERIVED(DerivedType) \
		public: \
			DECLARE_TYPE_NAME(DerivedType) \
			typedef RTTIBase Base; \
			typedef DerivedType RTTIBase; \
			/** Compare with selected type */ \
			virtual bool isType(RTTI_TYPE _type) const { return RTTI_GET_TYPE(DerivedType) == _type || Base::isType(_type); } \
			/** Compare with selected type */ \
			template<typename Type> bool isType() const { return isType(RTTI_GET_TYPE(Type)); }

#endif //SDL2GUITEST_SRC_FRAMEWORK_RTTI
