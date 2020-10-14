from setuptools import setup

package_name = 'scenario_test_utility'

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name + '/OpenSCENARIO.xsd']),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='tanaka-car',
    maintainer_email='ttatcoder@outlook.jp',
    description='scenario test utility package for autoware.auto',
    license='Apache License 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'scenario_converter = scenario_test_utility.scenario_converter:main',
            'xosc_validator = scenario_test_utility.xosc_validator:main'
        ],
    },
)