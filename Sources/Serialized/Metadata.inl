#pragma once

#include "Helpers/ConstExpr.hpp"
#include "Resources/Resource.hpp"
#include "Metadata.hpp"

namespace acid
{
template<typename T>
T Metadata::GetValue() const
{
	return String::From<T>(m_value);
}

template<typename T>
void Metadata::SetValue(const T &value)
{
	m_value = String::To(value);
}

template<typename T>
T Metadata::GetChild(const std::string &name) const
{
	auto child{FindChild(name)};

	if (child == nullptr)
	{
		return T{};
	}

	T result;
	*child >> result;
	return std::move(result);
}

template<typename T>
T Metadata::GetChildDefault(const std::string &name, const T &value)
{
	auto child{FindChild(name)};

	if (child == nullptr)
	{
		child = AddChild(std::make_unique<Metadata>(name));
		*child << value;
	}

	T result;
	*child >> result;
	return std::move(result);
}

template<typename T>
void Metadata::GetChild(const std::string &name, T &dest) const
{
	auto child{FindChild(name)};

	if (child == nullptr)
	{
		return;
	}

	*child >> dest;
}

template<typename T>
void Metadata::SetChild(const std::string &name, const T &value)
{
	auto child{FindChild(name)};

	if (child == nullptr)
	{
		child = AddChild(std::make_unique<Metadata>(name));
	}

	*child << value;
}

template<typename T>
const Metadata &operator>>(const Metadata &metadata, T &object)
{
	object = String::From<T>(metadata.GetValue());
	return metadata;
}

template<typename T>
Metadata &operator<<(Metadata &metadata, const T &object)
{
	metadata.SetValue(object);
	return metadata;
}

template<typename T>
const Metadata &operator>>(const Metadata &metadata, std::unique_ptr<T> &object)
{
	T x;
	metadata >> x;
	object = std::make_unique<T>(std::move(x));
	return metadata;
}

template<typename T>
Metadata &operator<<(Metadata &metadata, const std::unique_ptr<T> &object)
{
	if (object == nullptr)
	{
		metadata.SetValue("null");
		return metadata;
	}

	metadata << *object;
	return metadata;
}

template<typename T>
const Metadata &operator>>(const Metadata &metadata, std::shared_ptr<T> &object)
{
	// TODO: Abstract Resource streams out from shared_ptr.
	if constexpr (std::is_base_of_v<Resource, T>)
	{
		object = T::Create(metadata);
		return metadata;
	}
	else
	{
		auto x = new T();
		metadata >> *x;
		object = std::make_shared<T>(x);
		return metadata;
	}
}

template<typename T>
Metadata &operator<<(Metadata &metadata, const std::shared_ptr<T> &object)
{
	if (object == nullptr)
	{
		metadata.SetValue("null");
		return metadata;
	}

	metadata << *object;
	return metadata;
}

template<typename T>
const Metadata &operator>>(const Metadata &metadata, std::basic_string<T, std::char_traits<T>, std::allocator<T>> &string)
{
	string = metadata.GetValue();
	return metadata;
}

template<typename T>
Metadata &operator<<(Metadata &metadata, const std::basic_string<T, std::char_traits<T>, std::allocator<T>> &string)
{
	metadata.SetValue(string);
	return metadata;
}

inline const Metadata &operator>>(const Metadata &metadata, std::filesystem::path &object)
{
	object = metadata.GetValue();
	return metadata;
}

inline Metadata &operator<<(Metadata &metadata, const std::filesystem::path &object)
{
	metadata.SetValue(object.string());
	return metadata;
}

template<typename T>
std::enable_if_t<std::is_class_v<T> || std::is_pointer_v<T>, const Metadata &> operator>>(const Metadata &metadata, T &object)
{
	metadata >> ConstExpr::AsRef(object);
	return metadata;
}

template<typename T>
std::enable_if_t<std::is_class_v<T> || std::is_pointer_v<T>, Metadata &> operator<<(Metadata &metadata, const T &object)
{
	if (ConstExpr::AsPtr(object) == nullptr)
	{
		metadata.SetValue("null");
		return metadata;
	}

	metadata << ConstExpr::AsRef(object);
	return metadata;
}

template<typename T, typename K>
const Metadata &operator>>(const Metadata &metadata, std::pair<T, K> &pair)
{
	pair.first = String::From<T>(metadata.GetName());
	metadata >> pair.second;
	return metadata;
}

template<typename T, typename K>
Metadata &operator<<(Metadata &metadata, const std::pair<T, K> &pair)
{
	metadata.SetName(String::To(pair.first));
	metadata << pair.second;
	return metadata;
}

template<typename T>
const Metadata &operator>>(const Metadata &metadata, std::optional<T> &optional)
{
	if (metadata.GetValue() != "null")
	{
		T x;
		metadata >> x;
		optional = std::move(x);
	}
	else
	{
		optional = std::nullopt;
	}

	return metadata;
}

template<typename T>
Metadata &operator<<(Metadata &metadata, const std::optional<T> &optional)
{
	if (optional)
	{
		metadata << *optional;
	}
	else
	{
		metadata.SetValue("null");
		metadata.SetType(Metadata::Type::Null);
	}

	return metadata;
}

template<typename T>
const Metadata &operator>>(const Metadata &metadata, std::vector<T> &vector)
{
	vector = {};
	vector.reserve(metadata.GetChildren().size());

	for (const auto &child : metadata.GetChildren())
	{
		T x;
		*child >> x;
		vector.emplace_back(std::move(x));
	}

	return metadata;
}

template<typename T>
Metadata &operator<<(Metadata &metadata, const std::vector<T> &vector)
{
	for (const auto &x : vector)
	{
		auto child{metadata.AddChild(std::make_unique<Metadata>())};
		*child << x;
	}

	return metadata;
}

template<typename T, typename K>
const Metadata &operator>>(const Metadata &metadata, std::map<T, K> &map)
{
	map = {};

	for (const auto &child : metadata.GetChildren())
	{
		std::pair<T, K> pair;
		*child >> pair;
		map.emplace(std::move(pair));
	}

	return metadata;
}

template<typename T, typename K>
Metadata &operator<<(Metadata &metadata, const std::map<T, K> &map)
{
	for (const auto &x : map)
	{
		auto child{metadata.AddChild(std::make_unique<Metadata>())};
		*child << x;
	}

	return metadata;
}
}
